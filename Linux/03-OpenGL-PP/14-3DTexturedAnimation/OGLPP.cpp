//headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "vmath.h"
#include <SOIL/SOIL.h>

//namespaces
using namespace std;
using namespace vmath;

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

//type declarations
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

//global variable declarations
Display      *gpDisplay     = NULL;
XVisualInfo  *gpXVisualInfo = NULL;
Colormap      gColormap;
Window        gWindow;
GLXContext    gGLXContext;
GLXFBConfig   gGLXFBConfig;

glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;

int giWindowHeight = 600;
int giWindowWidth  = 800;
bool gbFullscreen  = false;

GLuint vertexShaderObject;         //handle to vertex shader object
GLuint fragmentShaderObject;       //handle to fragment shader object
GLuint shaderProgramObject;        //handle to shader program object

GLuint vao_pyramid;               //handle to vertex array object for pyramid
GLuint vbo_pyramid_position;      //handle to vertex buffer object for vertices of pyramid
GLuint vbo_pyramid_texture;       //handle to vertex buffer object for texture of pyramid

GLuint vao_cube;                  //handle to vertex array object for cube
GLuint vbo_cube_position;         //handle to vertex buffer object for vertices of cube
GLuint vbo_cube_texture;          //handle to vertex buffer object for texture of cube

GLuint mvpMatrixUniform;           
GLuint textureSamplerUniform;

mat4 perspectiveProjectionMatrix;  

GLuint stone_texture;
GLuint kundali_texture;

GLfloat pyramid_rotation_angle = 0.0f;
GLfloat cube_rotation_angle = 0.0f;

//entry-point function
int main(void)
{
    //function prototypes
    void CreateWindow(void);
    void ToggleFullscreen(void);
    void Initialize(void);
    void Resize(int, int);
    void Render(void);
    void uninitialize(void);
    
    //variable declarations
    int winWidth   = giWindowWidth;
    int winHeight  = giWindowHeight;
    bool bDone     = false;
    
    //code
    CreateWindow();
    
    Initialize();
    
    //Message Loop
    XEvent event;
    KeySym keysym;
    
    while(bDone == false)
    {
        while(XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);
            switch(event.type)
            {
                case MapNotify:
                    break;
            
                case KeyPress:
                    keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                    switch(keysym)
                    {
                        case XK_Escape:
                            bDone = true;
                            break;
                        
                        case XK_F:
                        case XK_f:
                            if(gbFullscreen == false)
                            {
                                ToggleFullscreen();
                                gbFullscreen = true;
                            }
                            else
                            {
                                ToggleFullscreen();
                                gbFullscreen = false;
                            }
                            break;
                    
                        default:
                            break;
                    }
                    break;
                        
                case ButtonPress:
                    switch(event.xbutton.button)
                    {
                        case 1:
                            break;
                            
                        case 2:
                            break;
                            
                        case 3:
                            break;
                            
                        default:
                            break;
                    }
                    break;
                        
                case MotionNotify:
                    break;
                    
                case ConfigureNotify:
                    winWidth = event.xconfigure.width;
                    winHeight = event.xconfigure.height;
                    Resize(winWidth, winHeight);
                    break;
                        
                case Expose:
                    break;
                    
                case DestroyNotify:
                    break;
                    
                case 33:
                    bDone = true;
                    break;
                    
                default:
                    break;
            }
        }
        
        Render();
    }
    
    uninitialize();
    
    return (0);
}

void CreateWindow(void)
{
    //function prototypes
    void uninitialize(void);
    
    //variable declarations
    XSetWindowAttributes winAttribs;
    GLXFBConfig *pGLXFBConfig = NULL;
    XVisualInfo *pTempXVisualInfo = NULL;
    int numFBConfigs = 0;
    
    int defaultScreen;
    int styleMask;
    
    static int frameBufferAttributes[] = { GLX_X_RENDERABLE,             True,
                                           GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                                           GLX_RENDER_TYPE,      GLX_RGBA_BIT,
                                           GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
                                           GLX_STENCIL_SIZE,                8,
                                           GLX_DOUBLEBUFFER,             True,
                                           GLX_RED_SIZE,                    8,
                                           GLX_GREEN_SIZE,                  8,
                                           GLX_BLUE_SIZE,                   8,
                                           GLX_ALPHA_SIZE,                  8,
                                           GLX_DEPTH_SIZE,                 24,
                                           None };
    
    //code
    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        printf("ERROR : Unable To Open X Display.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    defaultScreen = XDefaultScreen(gpDisplay);
    
    gpXVisualInfo = (XVisualInfo*)malloc(sizeof(XVisualInfo));
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR : Unable To Allocate Memory For Visual Info.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    pGLXFBConfig = glXChooseFBConfig(gpDisplay, defaultScreen, frameBufferAttributes, &numFBConfigs);
    if(pGLXFBConfig == NULL)
    {
        printf("ERROR : Unable To Find FBConfigs.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    printf("Number of framebuffer configurations found = %d\n", numFBConfigs);
    
    int bestFrameBufferConfig = -1;
    int worstFrameBufferConfig = -1;
    int bestNumberOfSamples = -1;
    int worstNumberOfSamples = 999;
    int i;
    
    for(i = 0; i < numFBConfigs; i++)
    {
        pTempXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfig[i]);
        if(pTempXVisualInfo)
        {
            int sampleBuffers;
            int samples;
           
            glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);            
            glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLES, &samples);
            
            if((bestFrameBufferConfig < 0) || sampleBuffers && (samples > bestNumberOfSamples)) 
            {
                bestFrameBufferConfig = i;
                bestNumberOfSamples = samples;
            }
            
            if((worstFrameBufferConfig < 0) || !sampleBuffers || (samples < worstNumberOfSamples))
            {
                worstFrameBufferConfig = i;
                worstNumberOfSamples = samples;
            }
            
            printf("FBConfig %d SampleBuffers %d Samples %d Visualid = %ld\n", i, sampleBuffers, samples, pTempXVisualInfo->visualid);
        }
        else
        {
            printf("ERROR : Failed TO Get A Visual From FBConfig.\nExiting Now...\n");
            uninitialize();
            exit(1);
        }
        
        XFree(pTempXVisualInfo);
        pTempXVisualInfo = NULL;
    }
    
    gGLXFBConfig = pGLXFBConfig[bestFrameBufferConfig];
    XFree(pGLXFBConfig);
    pGLXFBConfig = NULL;
    
    gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, gGLXFBConfig);
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR : Failed To Get A Visual From FBConfig.\n");
        uninitialize();
        exit(1);
    }
    
    winAttribs.border_pixel       = 0;
    winAttribs.background_pixmap  = 0;
    winAttribs.colormap           = XCreateColormap(gpDisplay, 
                                                    RootWindow(gpDisplay, gpXVisualInfo->screen),
                                                    gpXVisualInfo->visual, 
                                                    AllocNone);
    winAttribs.background_pixel   = BlackPixel(gpDisplay, defaultScreen);
    winAttribs.event_mask         = ExposureMask | VisibilityChangeMask | ButtonPressMask |
                                    KeyPressMask | PointerMotionMask | StructureNotifyMask;
                                    
    styleMask                     = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
    gColormap                     = winAttribs.colormap;
    
    gWindow = XCreateWindow(gpDisplay, 
                            RootWindow(gpDisplay, gpXVisualInfo->screen),
                            0,
                            0,
                            giWindowWidth,
                            giWindowHeight,
                            0,
                            gpXVisualInfo->depth,
                            InputOutput,
                            gpXVisualInfo->visual,
                            styleMask,
                            &winAttribs);
    if(!gWindow)
    {
        printf("ERROR : Failed To Create Main Window.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Texture");

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);
    
    XMapWindow(gpDisplay, gWindow);
}

void ToggleFullscreen(void)
{
    //variable declaration
    Atom wm_state;
    Atom fullscreen;
    XEvent xev = {0};
    
    //code
    wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    memset(&xev, 0, sizeof(xev));
    
    xev.type                  = ClientMessage;
    xev.xclient.window        = gWindow;
    xev.xclient.message_type  = wm_state;
    xev.xclient.format        = 32;
    xev.xclient.data.l[0]     = gbFullscreen ? 0 : 1;
    
    fullscreen                = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[1]     = fullscreen;
    
    XSendEvent(gpDisplay, 
               RootWindow(gpDisplay, gpXVisualInfo->screen),
               False,
               StructureNotifyMask,
               &xev);
}

void Initialize(void)
{
    //function declarations
    void Resize(int, int);
    void uninitialize(void);
    GLuint LoadBitmapAsTexture(const char* path);
    
    //code
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
    if(glXCreateContextAttribsARB == NULL)
    {
        printf("ERROR : Failed To Get glXCreateContextAttribsARB Address.\n");
        uninitialize();
        exit(1);
    }
    
    const int attribsCore[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                                GLX_CONTEXT_MINOR_VERSION_ARB, 5,
                                GLX_CONTEXT_PROFILE_MASK_ARB,  GL_CONTEXT_CORE_PROFILE_BIT,
                                None };
    
    gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig,  0, True, attribsCore);
    if(gGLXContext == NULL)
    {
        const int attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
                                GLX_CONTEXT_MINOR_VERSION_ARB, 1,
                                None };
                                    
        gGLXContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attribs);
    }
                            
    bool is_direct_context = glXIsDirect(gpDisplay, gGLXContext);
    if(is_direct_context)
    {
        printf("Rendering Context Is Hardware Rendering Context.\n");
    }
    else
    {
        printf("Rendering Context Is Software Rendering Context.\n");
    }
    
    glXMakeCurrent(gpDisplay, gWindow, gGLXContext);
    
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK)
    {
        printf("ERROR : glewInit() failed.\n");
        uninitialize();
        exit(1);
    }
    
    //OpenGL related log
    printf("\nOpenGL Information\n");
    printf("OpenGL Vendor     : %s\n", glGetString(GL_VENDOR));
    printf("OpenGL Renderer   : %s\n", glGetString(GL_RENDERER));
    printf("OpenGL Version    : %s\n", glGetString(GL_VERSION));
    printf("GLSL Version      : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    //OpenGL enabled extensions
    GLint numExt;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    
    printf("OpenGL Extensions : \n");
    for(int i = 0; i < numExt; i++)
    {
        printf("%s\n", glGetStringi(GL_EXTENSIONS, i));
    }
    printf("\n\n");
    
    //--- Vertex Shader ---

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                         \
        "\n"                                                        \
        "in vec4 vPosition;"                                        \
        "in vec2 vTexCoord;"                                        \
        "uniform mat4 u_mvpMatrix;"                                 \
        "out vec2 out_texcoord;"                                    \
        "void main(void)"                                           \
        "{"                                                         \
        "   gl_Position = u_mvpMatrix * vPosition;"                 \
        "   out_texcoord = vTexCoord;"                              \
        "}";

    //provide source code to shader object
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

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
                printf("Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    } 

    printf("Vertex Shader Compiled Successfully.\n");

    //--- Fragment Shader ---

    //create shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                              \
        "\n"                                                             \
        "in vec2 out_texcoord;"                                          \
        "out vec4 fragColor;"                                            \
        "uniform sampler2D u_textureSampler;"                            \
        "void main(void)"                                                \
        "{"                                                              \
        "   fragColor = texture(u_textureSampler, out_texcoord);"        \
        "}";

    //provide source code to shader object 
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

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
                printf("Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    }

    printf("Fragment Shader Compiled Successfully.\n");

    //--- Shader Program ---

    //create shader program
    shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(shaderProgramObject, vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader texture attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");

    //link shader program 
    glLinkProgram(shaderProgramObject);

    //shader linking error checking
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
                printf("Shader Program Link Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    }

    printf("Shader Program Linked Successfully.\n");

    //get MVP uniform location
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix"); 

    //get uniform sampler2D location
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "u_textureSampler");

    //pyramid vertex, texcoord data
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

    const GLfloat pyramidTexcoords[] =
    {
        //near
        0.5f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        //right
        0.5f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        //far
        0.5f, 1.0f, 
        0.0f, 0.0f,
        1.0f, 0.0f,

        //left
        0.5f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    //cube vertex, texcoord data
    const GLfloat cubeVertices[] = 
    {
        //near 
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 
        1.0f, -1.0f, 1.0f,

        //right
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,

        //far
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        //left
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f, 
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,

        //top
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        //bottom
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f
    };
 

    const GLfloat cubeTexcoords[] = 
    {
        //near
        1.0f, 1.0f, 
        0.0f, 1.0f,
        0.0f, 0.0f, 
        1.0f, 0.0f,

        //right
        1.0f, 1.0f, 
        0.0f, 1.0f,
        0.0f, 0.0f, 
        1.0f, 0.0f,

        //far 
        1.0f, 1.0f, 
        0.0f, 1.0f,
        0.0f, 0.0f, 
        1.0f, 0.0f,

        //left
        1.0f, 1.0f, 
        0.0f, 1.0f,
        0.0f, 0.0f, 
        1.0f, 0.0f,

        //top
        1.0f, 1.0f, 
        0.0f, 1.0f,
        0.0f, 0.0f, 
        1.0f, 0.0f,

        //bottom
        1.0f, 1.0f, 
        0.0f, 1.0f,
        0.0f, 0.0f, 
        1.0f, 0.0f,
    };

    //setup vao and vbo for triangle
    glGenVertexArrays(1, &vao_pyramid);
    glBindVertexArray(vao_pyramid);

    glGenBuffers(1, &vbo_pyramid_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glGenBuffers(1, &vbo_pyramid_texture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_texture);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidTexcoords), pyramidTexcoords, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

    //unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //setup vao and vbo for square
    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);

    glGenBuffers(1, &vbo_cube_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glGenBuffers(1, &vbo_cube_texture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texture);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexcoords), cubeTexcoords, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

    //unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glShadeModel(GL_SMOOTH);
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    perspectiveProjectionMatrix = mat4::identity();
    
    kundali_texture = LoadBitmapAsTexture("textures/Vijay_Kundali.bmp");
    stone_texture = LoadBitmapAsTexture("textures/Stone.bmp");
    
    glEnable(GL_TEXTURE_2D);
    
    Resize(giWindowWidth, giWindowHeight);
}

GLuint LoadBitmapAsTexture(const char* path)
{
    //variable declarations
    int width, height;
    unsigned char* image_data = NULL;
    GLuint texture_id;
    
    //code
    image_data = SOIL_load_image(path, &width, &height, NULL, SOIL_LOAD_RGB);
    
    //generate texture object
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    //set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    //push the data to the texture memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    //release image
    SOIL_free_image_data(image_data);
    image_data = NULL;
    
    return (texture_id);
}

void Resize(int width, int height)
{
    //code
    if(height == 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void Render(void)
{
    //variable declarations
    mat4 modelViewMatrix;
    mat4 modelViewProjectionMatrix;
    mat4 translateMatrix;
    mat4 rotationMatrix_x;
    mat4 rotationMatrix_y;
    mat4 rotationMatrix_z;
    mat4 scaleMatrix;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //start using OpenGL program object
    glUseProgram(shaderProgramObject);

    //OpenGL Drawing
    
    //--- Pyramid ---

    //set modelview, modelviewprojection & translate matrices to identity
    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    translateMatrix = mat4::identity();
    rotationMatrix_y = mat4::identity();
    
    //translate and rotate modelview matrix
    translateMatrix = vmath::translate(-1.5f, 0.0f, -6.0f);
    rotationMatrix_y = vmath::rotate(pyramid_rotation_angle, 0.0f, 1.0f, 0.0f);
    modelViewMatrix = translateMatrix * rotationMatrix_y;

    //multiply the modelview and perspective projection matrix to get modelviewprojection matrix 
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    //pass above modelviewprojection matrix to the vertex shader in
    //"u_mvpMatrix" shader variable
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    //bind stone texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stone_texture);
    glUniform1i(textureSamplerUniform, 0);
    
    //bind vao for pyramid
    glBindVertexArray(vao_pyramid);

    //draw
    glDrawArrays(GL_TRIANGLES, 0, 12);

    //unbind vao
    glBindVertexArray(0);

    //--- Cube ---

    //set modelview, modelviewprojection and translate matrix to identity
    modelViewMatrix = mat4::identity();
    modelViewProjectionMatrix = mat4::identity();
    translateMatrix = mat4::identity();
    rotationMatrix_x = mat4::identity();
    rotationMatrix_y = mat4::identity();
    rotationMatrix_z = mat4::identity();
    scaleMatrix = mat4::identity();

    //translate modelview matrix
    translateMatrix = vmath::translate(1.5f, 0.0f, -6.0f);
    scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);
    rotationMatrix_x = vmath::rotate(cube_rotation_angle, 1.0f, 0.0f, 0.0f);
    rotationMatrix_y = vmath::rotate(cube_rotation_angle, 0.0f, 1.0f, 0.0f);
    rotationMatrix_z = vmath::rotate(cube_rotation_angle, 0.0f, 0.0f, 1.0f);
    modelViewMatrix = translateMatrix * scaleMatrix * rotationMatrix_x * rotationMatrix_y * rotationMatrix_z;

    //multiply the modelview and perspective projection matrix to get modelviewprojection matrix 
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;

    //pass above modelviewprojection matrix to the vertex shader in
    //"u_mvpMatrix" shader variable
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    //bind kundali texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, kundali_texture);
    glUniform1i(textureSamplerUniform, 0);
    
    //bind vao for cube
    glBindVertexArray(vao_cube);

    //draw
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

    //unbind vao 
    glBindVertexArray(0);

    //stop using OpenGL program object
    glUseProgram(0);

    //update 
    pyramid_rotation_angle += 0.01f;
    if(pyramid_rotation_angle >= 360.0f)
        pyramid_rotation_angle = 0.0f;
    
    cube_rotation_angle += 0.01f;
    if(cube_rotation_angle >= 360.0f)
        cube_rotation_angle = 0.0f;
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void uninitialize(void)
{
    //variable declarations
    GLXContext currentGLXContext;
    
    //code
    //delete textures
    glDeleteTextures(1, &stone_texture);
    glDeleteTextures(1, &kundali_texture);

    //release vao & vbo for triangle
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

    if(vbo_pyramid_texture)
    {
        glDeleteBuffers(1, &vbo_pyramid_texture);
        vbo_pyramid_texture = 0;
    }

    //release vao and vbo for square
    if(vao_cube)
    {
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
    }

    if(vbo_cube_position)
    {
        glDeleteBuffers(1, &vbo_cube_position);
        vbo_cube_position = 0;
    }

    if(vbo_cube_texture)
    {
        glDeleteBuffers(1, &vbo_cube_texture);
        vbo_cube_texture = 0;
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
    
    currentGLXContext = glXGetCurrentContext();
    if(currentGLXContext == gGLXContext)
    {
        glXMakeCurrent(gpDisplay, 0, 0);
    }
    
    if(gGLXContext)
    {
        glXDestroyContext(gpDisplay, gGLXContext);
    }
    
    if(gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
    }
    
    if(gColormap)
    {
        XFreeColormap(gpDisplay, gColormap);
    }
    
    if(gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }
    
    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }
}




