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
#include "Sphere.h"

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

struct LIGHT
{
    vec4 lightAmbient;
    vec4 lightDiffuse;
    vec4 lightSpecular;
    vec4 lightPosition;
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

GLuint v_vertexShaderObject;        //handle to vertex shader object
GLuint v_fragmentShaderObject;      //handle to fragment shader object
GLuint v_shaderProgramObject;       //handle to shader program object

GLuint f_vertexShaderObject;        //handle to vertex shader object
GLuint f_fragmentShaderObject;      //handle to fragment shader object
GLuint f_shaderProgramObject;       //handle to shader program object

GLuint vao_sphere;                  //handle to vertex array object for sphere
GLuint vbo_sphere_position;         //handle to vertex buffer object for vertices of sphere
GLuint vbo_sphere_normal;           //handle to vertex buffer object for normals of sphere
GLuint vbo_sphere_texcoord;         //handle to vertex buffer object for texcoords of sphere
GLuint vbo_sphere_elements;         //handle to vertex buffer object for indices 

GLuint v_modelMatrixUniform;
GLuint v_viewMatrixUniform;
GLuint v_perspectiveProjectionMatrixUniform;
GLuint v_LaUniform[3];
GLuint v_LdUniform[3];
GLuint v_LsUniform[3];
GLuint v_lightPositionUniform[3];
GLuint v_KaUniform;
GLuint v_KdUniform;
GLuint v_KsUniform;
GLuint v_materialShininessUniform;
GLuint v_LKeyPressedUniform;

GLuint f_modelMatrixUniform;
GLuint f_viewMatrixUniform;
GLuint f_perspectiveProjectionMatrixUniform;
GLuint f_LaUniform[3];
GLuint f_LdUniform[3];
GLuint f_LsUniform[3];
GLuint f_lightPositionUniform[3];
GLuint f_KaUniform;
GLuint f_KdUniform;
GLuint f_KsUniform;
GLuint f_materialShininessUniform;
GLuint f_LKeyPressedUniform;
      
mat4 perspectiveProjectionMatrix;  

struct LIGHT light[3];

vec4 La;
vec4 Ld;
vec4 Ls;
vec4 lightPosition;

vec4 Ka;
vec4 Kd;
vec4 Ks;
float materialShininess;

int LKeyPressed;
bool perVertex_perFragmentToggle;

GLfloat sphere_vertices[1146];
GLfloat sphere_normals[1146];
GLfloat sphere_texcoords[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

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
                        case XK_Q:
                        case XK_q:
                            bDone = true;
                            break;
                            
                        case XK_Escape:
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
                    
                        case XK_L:
                        case XK_l:
                            if(LKeyPressed == 0)
                            {
                                LKeyPressed = 1;
                            }
                            else
                            {
                                LKeyPressed = 0;
                            }
                            break;
                        
                        case XK_V:
                        case XK_v:
                            perVertex_perFragmentToggle = 0;
                            break;
                            
                        case XK_F:
                        case XK_f:
                            perVertex_perFragmentToggle = 1;
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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Three Moving Lights");

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
    
    //setup render scene

    //--- Per Vertex Lighting ---
    
    printf("***** Per Vertex Lighting *****\n");

    //vertex shader

    //create shader
    v_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* v_vertexShaderSourceCode = 
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

    //provide source code to shader object
    glShaderSource(v_vertexShaderObject, 1, (const GLchar**)&v_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(v_vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

    glGetShaderiv(v_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(v_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(v_vertexShaderObject, infoLogLength, &written, szInfoLog);
                printf("Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    } 

    printf("\nVertex Shader Compiled Successfully.\n");

    //fragment shader

    //create shader
    v_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* v_fragmentShaderSourceCode = 
        "#version 450 core"                                 \
        "\n"                                                \
        "in vec3 phong_ads_light;"                          \
        "out vec4 FragColor;"                               \
        "void main(void)"                                   \
        "{"                                                 \
        "   FragColor = vec4(phong_ads_light, 1.0f);"       \
        "}";

    //provide source code to shader object 
    glShaderSource(v_fragmentShaderObject, 1, (const GLchar**)&v_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(v_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(v_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(v_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(v_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                printf("Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    }

    printf("Fragment Shader Compiled Successfully.\n");

    //shader program 

    //create shader program
    v_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(v_shaderProgramObject, v_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(v_shaderProgramObject, v_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(v_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(v_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link shader program 
    glLinkProgram(v_shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(v_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(v_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(v_shaderProgramObject, infoLogLength, &written, szInfoLog);
                printf("Shader Program Link Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    }

    printf("Shader Program Linked Successfully.\n");

    //get uniform locations
    v_modelMatrixUniform = glGetUniformLocation(v_shaderProgramObject, "u_modelMatrix");
    v_viewMatrixUniform = glGetUniformLocation(v_shaderProgramObject, "u_viewMatrix");
    v_perspectiveProjectionMatrixUniform = glGetUniformLocation(v_shaderProgramObject, "u_perspectiveProjectionMatrix");

    v_LaUniform[0] = glGetUniformLocation(v_shaderProgramObject, "u_La[0]");
    v_LdUniform[0] = glGetUniformLocation(v_shaderProgramObject, "u_Ld[0]");
    v_LsUniform[0] = glGetUniformLocation(v_shaderProgramObject, "u_Ls[0]");
    v_lightPositionUniform[0] = glGetUniformLocation(v_shaderProgramObject, "u_lightPosition[0]");

    v_LaUniform[1] = glGetUniformLocation(v_shaderProgramObject, "u_La[1]");
    v_LdUniform[1] = glGetUniformLocation(v_shaderProgramObject, "u_Ld[1]");
    v_LsUniform[1] = glGetUniformLocation(v_shaderProgramObject, "u_Ls[1]");
    v_lightPositionUniform[1] = glGetUniformLocation(v_shaderProgramObject, "u_lightPosition[1]");

    v_LaUniform[2] = glGetUniformLocation(v_shaderProgramObject, "u_La[2]");
    v_LdUniform[2] = glGetUniformLocation(v_shaderProgramObject, "u_Ld[2]");
    v_LsUniform[2] = glGetUniformLocation(v_shaderProgramObject, "u_Ls[2]");
    v_lightPositionUniform[2] = glGetUniformLocation(v_shaderProgramObject, "u_lightPosition[2]");

    v_KaUniform = glGetUniformLocation(v_shaderProgramObject, "u_Ka");
    v_KdUniform = glGetUniformLocation(v_shaderProgramObject, "u_Kd");
    v_KsUniform = glGetUniformLocation(v_shaderProgramObject, "u_Ks");
    v_materialShininessUniform = glGetUniformLocation(v_shaderProgramObject, "u_materialShininess");

    v_LKeyPressedUniform = glGetUniformLocation(v_shaderProgramObject, "u_LKeyPressed");

    //--- Per Fragment Lighting ---

    printf("\n***** Per Fragment Lighting *****\n");

    //vertex shader

    //create shader
    f_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* f_vertexShaderSourceCode = 
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

    //provide source code to shader object
    glShaderSource(f_vertexShaderObject, 1, (const GLchar**)&f_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(f_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(f_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(f_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(f_vertexShaderObject, infoLogLength, &written, szInfoLog);
                printf("Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    } 

    printf("\nVertex Shader Compiled Successfully.\n");

    //--- Fragment Shader ---

    //create shader
    f_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* f_fragmentShaderSourceCode = 
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

    //provide source code to shader object 
    glShaderSource(f_fragmentShaderObject, 1, (const GLchar**)&f_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(f_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(f_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(f_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(f_fragmentShaderObject, infoLogLength, &written, szInfoLog);
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
    f_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(f_shaderProgramObject, f_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(f_shaderProgramObject, f_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(f_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(f_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link shader program 
    glLinkProgram(f_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(f_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(f_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(f_shaderProgramObject, infoLogLength, &written, szInfoLog);
                printf("Shader Program Link Log : %s\n", szInfoLog);
                free(szInfoLog);
                uninitialize();
                exit(1);
            }
        }
    }

    printf("Shader Program Linked Successfully\n");

    //get uniform locations
    f_modelMatrixUniform = glGetUniformLocation(f_shaderProgramObject, "u_modelMatrix");
    f_viewMatrixUniform = glGetUniformLocation(f_shaderProgramObject, "u_viewMatrix");
    f_perspectiveProjectionMatrixUniform = glGetUniformLocation(f_shaderProgramObject, "u_perspectiveProjectionMatrix");

    f_LaUniform[0] = glGetUniformLocation(f_shaderProgramObject, "u_La[0]");
    f_LdUniform[0] = glGetUniformLocation(f_shaderProgramObject, "u_Ld[0]");
    f_LsUniform[0] = glGetUniformLocation(f_shaderProgramObject, "u_Ls[0]");
    f_lightPositionUniform[0] = glGetUniformLocation(f_shaderProgramObject, "u_lightPosition[0]");

    f_LaUniform[1] = glGetUniformLocation(f_shaderProgramObject, "u_La[1]");
    f_LdUniform[1] = glGetUniformLocation(f_shaderProgramObject, "u_Ld[1]");
    f_LsUniform[1] = glGetUniformLocation(f_shaderProgramObject, "u_Ls[1]");
    f_lightPositionUniform[1] = glGetUniformLocation(f_shaderProgramObject, "u_lightPosition[1]");

    f_LaUniform[2] = glGetUniformLocation(f_shaderProgramObject, "u_La[2]");
    f_LdUniform[2] = glGetUniformLocation(f_shaderProgramObject, "u_Ld[2]");
    f_LsUniform[2] = glGetUniformLocation(f_shaderProgramObject, "u_Ls[2]");
    f_lightPositionUniform[2] = glGetUniformLocation(f_shaderProgramObject, "u_lightPosition[2]");

    f_KaUniform = glGetUniformLocation(f_shaderProgramObject, "u_Ka");
    f_KdUniform = glGetUniformLocation(f_shaderProgramObject, "u_Kd");
    f_KsUniform = glGetUniformLocation(f_shaderProgramObject, "u_Ks");
    f_materialShininessUniform = glGetUniformLocation(f_shaderProgramObject, "u_materialShininess");

    f_LKeyPressedUniform = glGetUniformLocation(f_shaderProgramObject, "u_LKeyPressed");

    //get buffer data for sphere
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_texcoords, sphere_elements);

    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    //setup vao and vbo for square
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

    glGenBuffers(1, &vbo_sphere_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_texcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_texcoords), sphere_texcoords, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);

    //unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //smooth shading  
    glShadeModel(GL_SMOOTH);                  

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();

    //initialize light properties
    light[0].lightAmbient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light[0].lightDiffuse = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    light[0].lightSpecular = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    light[0].lightPosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    light[1].lightAmbient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light[1].lightDiffuse = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    light[1].lightSpecular = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    light[1].lightPosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    light[2].lightAmbient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light[2].lightDiffuse = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    light[2].lightSpecular = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    light[2].lightPosition = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    //initialize material properties
    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Ks = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    materialShininess = 128.0f;
    
    Resize(giWindowWidth, giWindowHeight);
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
    mat4 modelMatrix;
    mat4 viewMatrix;

    const GLfloat radius = 5.0f;
    static GLfloat lightAngle = 0.0f;

    //code
    //clear the color buffer and depth buffer with currrent 
    //clearing values (set up in initilaize)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    //translate model matrix
    modelMatrix = vmath::translate(0.0f, 0.0f, -3.0f);

    //set light position
    light[0].lightPosition = vec4(0.0f, radius * sinf(lightAngle), radius * cosf(lightAngle), 1.0f);
    light[1].lightPosition = vec4(radius * sinf(lightAngle), 0.0f, radius * cosf(lightAngle), 1.0f);
    light[2].lightPosition = vec4(radius * sinf(lightAngle), radius * cosf(lightAngle), 0.0f, 1.0f);

    if(perVertex_perFragmentToggle == false)
    {
        glUseProgram(v_shaderProgramObject);

        //pass the uniform variables to shaders 
        if(LKeyPressed == 1)
        {
            glUniform1i(v_LKeyPressedUniform, 1);
            
            glUniform3fv(v_LaUniform[0], 1, light[0].lightAmbient);
            glUniform3fv(v_LdUniform[0], 1, light[0].lightDiffuse);
            glUniform3fv(v_LsUniform[0], 1, light[0].lightSpecular);
            glUniform4fv(v_lightPositionUniform[0], 1, light[0].lightPosition);
            
            glUniform3fv(v_LaUniform[1], 1, light[1].lightAmbient);
            glUniform3fv(v_LdUniform[1], 1, light[1].lightDiffuse);
            glUniform3fv(v_LsUniform[1], 1, light[1].lightSpecular);
            glUniform4fv(v_lightPositionUniform[1], 1, light[1].lightPosition);
            
            glUniform3fv(v_LaUniform[2], 1, light[2].lightAmbient);
            glUniform3fv(v_LdUniform[2], 1, light[2].lightDiffuse);
            glUniform3fv(v_LsUniform[2], 1, light[2].lightSpecular);
            glUniform4fv(v_lightPositionUniform[2], 1, light[2].lightPosition);
            
            glUniform3fv(v_KaUniform, 1, Ka);
            glUniform3fv(v_KdUniform, 1, Kd);
            glUniform3fv(v_KsUniform, 1, Ks);
            glUniform1f(v_materialShininessUniform, materialShininess);
        }
        else
        {
            glUniform1i(v_LKeyPressedUniform, 0);
        }

        //pass model, view and projection matrices to shader uniform variables
        glUniformMatrix4fv(v_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(v_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(v_perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    }
    else
    {
        glUseProgram(f_shaderProgramObject);

        //pass the uniform variables to shaders 
        if(LKeyPressed == 1)
        {
            glUniform1i(f_LKeyPressedUniform, 1);

            glUniform3fv(f_LaUniform[0], 1, light[0].lightAmbient);
            glUniform3fv(f_LdUniform[0], 1, light[0].lightDiffuse);
            glUniform3fv(f_LsUniform[0], 1, light[0].lightSpecular);
            glUniform4fv(f_lightPositionUniform[0], 1, light[0].lightPosition);

            glUniform3fv(f_LaUniform[1], 1, light[1].lightAmbient);
            glUniform3fv(f_LdUniform[1], 1, light[1].lightDiffuse);
            glUniform3fv(f_LsUniform[1], 1, light[1].lightSpecular);
            glUniform4fv(f_lightPositionUniform[1], 1, light[1].lightPosition);

            glUniform3fv(f_LaUniform[2], 1, light[2].lightAmbient);
            glUniform3fv(f_LdUniform[2], 1, light[2].lightDiffuse);
            glUniform3fv(f_LsUniform[2], 1, light[2].lightSpecular);
            glUniform4fv(f_lightPositionUniform[2], 1, light[2].lightPosition);

            glUniform3fv(f_KaUniform, 1, Ka);
            glUniform3fv(f_KdUniform, 1, Kd);
            glUniform3fv(f_KsUniform, 1, Ks);
            glUniform1f(f_materialShininessUniform, materialShininess);
        }
        else
        {
            glUniform1i(f_LKeyPressedUniform, 0);
        }

        //pass model, view and projection matrices to shader uniform variables
        glUniformMatrix4fv(f_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(f_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(f_perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    }

    //bind vao
    glBindVertexArray(vao_sphere);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    //unbind 
    glBindVertexArray(0);
    glUseProgram(0);

    //update 
    lightAngle += 0.0005f;
    if(lightAngle >= 360.0f)
        lightAngle = 0.0f;
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void uninitialize(void)
{
    //variable declarations
    GLXContext currentGLXContext;
    
    //code
    //release vao and vbo for square
    if(vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }

    if(vbo_sphere_position)
    {
        glDeleteBuffers(1, &vbo_sphere_position);
        vbo_sphere_position = 0;
    }

    if(vbo_sphere_normal)
    {
        glDeleteBuffers(1, &vbo_sphere_normal);
        vbo_sphere_normal = 0;
    }

    if(vbo_sphere_texcoord)
    {
        glDeleteBuffers(1, &vbo_sphere_texcoord);
        vbo_sphere_texcoord = 0;
    }

    //safe shader cleanup of per vertex lighting 
    if(v_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(v_shaderProgramObject);
        glGetProgramiv(v_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(v_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(v_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(v_shaderProgramObject);
        v_shaderProgramObject = 0;
        glUseProgram(0);
    }

    //safe shader cleanup of per vertex lighting 
    if(f_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(f_shaderProgramObject);
        glGetProgramiv(f_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(f_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(f_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(f_shaderProgramObject);
        f_shaderProgramObject = 0;
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




