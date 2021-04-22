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

GLuint vao_sphere;                 //handle to vertex array object for sphere
GLuint vbo_sphere_position;        //handle to vertex buffer object for vertices of sphere
GLuint vbo_sphere_normal;          //handle to vertex buffer object for normals of sphere
GLuint vbo_sphere_texcoord;        //handle to vertex buffer object for texcoords of sphere
GLuint vbo_sphere_elements;        //handle to vertex buffer object for indices 

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint perspectiveProjectionMatrixUniform;
GLuint LaUniform;
GLuint LdUniform;
GLuint LsUniform;
GLuint lightPositionUniform;
GLuint KaUniform;
GLuint KdUniform;
GLuint KsUniform;
GLuint materialShininessUniform;
GLuint LKeyPressedUniform;
               
mat4 perspectiveProjectionMatrix;  

vec4 La;
vec4 Ld;
vec4 Ls;
vec4 lightPosition;

vec4 Ka;
vec4 Kd;
vec4 Ks;
float materialShininess;

int LKeyPressed;

GLfloat sphere_vertices[1146];
GLfloat sphere_normals[1146];
GLfloat sphere_texcoords[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

GLfloat angle_for_x_rotation;
GLfloat angle_for_y_rotation;
GLfloat angle_for_z_rotation;

int key_pressed;
int width;
int height;

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
                            
                        case XK_X:
                        case XK_x:
                            key_pressed = 1;
                            angle_for_x_rotation = 0.0f;
                            break;
                            
                        case XK_Y:
                        case XK_y:
                            key_pressed = 2;
                            angle_for_y_rotation = 0.0f;
                            break;
                            
                        case XK_Z:
                        case XK_z:
                            key_pressed = 3;
                            angle_for_z_rotation = 0.0f;
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
                    width = winWidth;
                    height = winHeight;
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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : 24 Sphere");

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

    //--- Vertex Shader ---

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                                                     \
        "\n"                                                                                                    \
        "in vec4 vPosition;"                                                                                    \
        "in vec3 vNormal;"                                                                                      \
        "uniform mat4 u_modelMatrix;"                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \
        "uniform vec4 u_lightPosition;"                                                                         \
        "uniform int u_LKeyPressed;"                                                                            \
        "out vec3 transformed_normal;"                                                                          \
        "out vec3 light_direction;"                                                                             \
        "out vec3 view_vector;"                                                                                 \
        "void main(void)"                                                                                       \
        "{"                                                                                                     \
        "   if(u_LKeyPressed == 1)"                                                                             \
        "   {"                                                                                                  \
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                    \
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix))); "                  \
        "       transformed_normal = normal_matrix * vNormal;"                                                  \
        "       light_direction = vec3(u_lightPosition - eye_coords);"                                          \
        "       view_vector = -eye_coords.xyz;"                                                                 \
        "   }"                                                                                                  \
        "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"            \
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

    printf("\nVertex Shader Compiled Successfully.\n");

    //--- Fragment Shader ---

    //create shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                                                                             \
        "\n"                                                                                                                            \
        "in vec3 transformed_normal;"                                                                                                   \
        "in vec3 light_direction;"                                                                                                      \
        "in vec3 view_vector;"                                                                                                          \
        "uniform vec3 u_La;"                                                                                                            \
        "uniform vec3 u_Ld;"                                                                                                            \
        "uniform vec3 u_Ls;"                                                                                                            \
        "uniform vec3 u_Ka;"                                                                                                            \
        "uniform vec3 u_Kd;"                                                                                                            \
        "uniform vec3 u_Ks;"                                                                                                            \
        "uniform float u_materialShininess;"                                                                                            \
        "uniform int u_LKeyPressed;"                                                                                                    \
        "out vec4 fragColor;"                                                                                                           \
        "void main(void)"                                                                                                               \
        "{"                                                                                                                             \
        "   vec3 phong_ads_light;"                                                                                                      \
        "   if(u_LKeyPressed == 1)"                                                                                                       \
        "   {"                                                                                                                          \
        "       vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                    \
        "       vec3 normalized_view_vector = normalize(view_vector);"                                                                  \
        "       vec3 normalized_light_direction = normalize(light_direction);"                                                          \
        "       vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"                          \
        "       vec3 ambient = u_La * u_Ka;"                                                                                            \
        "       vec3 diffuse = u_Ld * u_Kd  * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"               \
        "       vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), u_materialShininess);"     \
        "       phong_ads_light = ambient + diffuse + specular;"                                                                        \
        "   }"                                                                                                                          \
        "   else"                                                                                                                       \
        "   {"                                                                                                                          \
        "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                              \
        "   }"                                                                                                                          \
        "   fragColor = vec4(phong_ads_light, 1.0f);"                                                                                   \
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

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

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

    //get uniform locations
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    perspectiveProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_perspectiveProjectionMatrix");

    LaUniform = glGetUniformLocation(shaderProgramObject, "u_La");
    LdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
    LsUniform = glGetUniformLocation(shaderProgramObject, "u_Ls");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");

    KaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
    KdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
    KsUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

    LKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");

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
    La = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Ld = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Ls = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightPosition = vec4(0.0f, 3.0f, 3.0f, 0.0f);
    
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
    //function declarations
    void DrawSpheres(void);

    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;

    GLfloat xCenter;
    GLfloat yCenter;

    //code
    //clear the color buffer and depth buffer with currrent 
    //clearing values (set up in initilaize)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    switch(key_pressed)
    {
        case 1:
            lightPosition[0] = 20.0f * sin(angle_for_x_rotation);
            lightPosition[1] = 20.0f * cos(angle_for_x_rotation);
            lightPosition[2] = 0.0f;
            break;
        
        case 2:
            lightPosition[0] = 20.0f * sin(angle_for_y_rotation);
            lightPosition[1] = 0.0f;
            lightPosition[2] = 20.0f * cos(angle_for_y_rotation);
            break;
        
        case 3:
            lightPosition[0] = 0.0f;
            lightPosition[1] = 20.0f * sin(angle_for_z_rotation);
            lightPosition[2] = 20.0f * cos(angle_for_z_rotation);
            break;
        
        default:
            break;
    }

    //pass the uniform variables to vertex shader
    if(LKeyPressed == 1)
    {
        glUniform1i(LKeyPressedUniform, 1);

        glUniform3fv(LaUniform, 1, La);
        glUniform3fv(LdUniform, 1, Ld);
        glUniform3fv(LsUniform, 1, Ls);
        glUniform4fv(lightPositionUniform, 1, lightPosition);
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    if(width <= height)
    {
        xCenter = 15.5f / 2.0f;
        yCenter = (15.5f * (GLfloat)height * 4.0f / (GLfloat)(width * 6.0f)) / 2.0f;
    }
    else
    {
        xCenter = (15.5f * (GLfloat)width * 6.0f / (GLfloat)(height * 4.0f)) / 2.0f;
        yCenter = 15.5f / 2.0f;
    }

    //translate model matrix
    modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

    //pass model, view and projection matrices to shader uniform variables
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    DrawSpheres();

    //unbind 
    glUseProgram(0);

    //update
    if(key_pressed == 1)
    {
        angle_for_x_rotation += 0.001f;
    }
    else if(key_pressed == 2)
    {
        angle_for_y_rotation += 0.001f;
    }
    else if(key_pressed == 3)
    {
        angle_for_z_rotation += 0.001f;
    }
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void DrawSpheres(void)
{
    //variable declarations
    mat4 modelMatrix;

    //code
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Resize(width / 4, height / 6);

    //bind vao
    glBindVertexArray(vao_sphere);

    // --- Emerald ---
    glViewport(0, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.0215f, 0.1745f, 0.0215f, 1.0f);
    Kd = vec4(0.07568f, 0.61424f, 0.07568f, 1.0f);
    Ks = vec4(0.633, 0.727811f, 0.633f, 1.0f);
    materialShininess   = 0.6f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Jade ---
    glViewport(width / 4, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
    Kd = vec4(0.54f, 0.89f, 0.63f, 1.0f);
    Ks = vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
    materialShininess = 0.1f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Obsidian ---
    glViewport(width * 2 / 4, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.05375f, 0.05f, 0.06625f, 1.0f);
    Kd = vec4(0.18275f, 0.17f, 0.22525f, 1.0f);
    Ks = vec4(0.332741f, 0.328634f, 0.346435f, 1.0f);
    materialShininess = 0.3f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Pearl ---
    glViewport(width * 3 / 4, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.25f, 0.20725f, 0.20725f, 1.0f);
    Kd = vec4(1.0f, 0.829f, 0.829f, 1.0f);
    Ks = vec4(0.296648f, 0.296648f, 0.296648f, 1.0f);
    materialShininess = 0.088f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Ruby ---
    glViewport(0, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
    Kd = vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
    Ks = vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
    materialShininess   = 0.6f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Turquoise ---
    glViewport(width / 4, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.1f, 0.18725f, 0.1745f, 1.0f);
    Kd = vec4(0.396f, 0.74151f, 0.69102f, 1.0f);
    Ks = vec4(0.297254f, 0.30829f, 0.306678f, 1.0f);
    materialShininess = 0.1f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Brass ---
    glViewport(width * 2 / 4, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
    Kd = vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
    Ks = vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
    materialShininess = 0.21794872f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Bronze ---
    glViewport(width * 3 / 4, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.2125f, 0.1275f, 0.054f, 1.0f);
    Kd = vec4(0.714f, 0.4284f, 0.18144f, 1.0f);
    Ks = vec4(0.393548f, 0.271906f, 0.166721f, 1.0f);
    materialShininess  = 0.2f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Chrome ---
    glViewport(0, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.25f, 0.25f, 0.25f, 1.0f);
    Kd = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    Ks = vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
    materialShininess = 0.6f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Copper ---
    glViewport(width / 4, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
    Kd = vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
    Ks = vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
    materialShininess = 0.1f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Gold ---
    glViewport(width * 2 / 4, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    Kd = vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
    Ks = vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
    materialShininess = 0.4f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Silver ---
    glViewport(width * 3 / 4, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.19225f, 0.19225f, 0.19225f, 1.0f);
    Ks = vec4(0.50754f, 0.50754f, 0.50754f, 1.0f);
    Kd = vec4(0.508273f, 0.508273f, 0.508273f, 1.0f);
    materialShininess  = 0.4f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Black ---
    glViewport(0, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.01f, 0.01f, 0.01f, 1.0f);
    Ks = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Cyan ---
    glViewport(width / 4, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.1f, 0.06f, 1.0f);
    Kd = vec4(0.0f, 0.50980392f, 0.50980392f, 1.0f);
    Ks = vec4(0.50196078f, 0.50196078f, 0.50196078f, 1.0f);
    materialShininess = 0.25f * 128.0f;    

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Green ---
    glViewport(width * 2 / 4, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.1f, 0.35f, 0.1f, 1.0f);
    Ks = vec4(0.45f, 0.55f, 0.45f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Red ---
    glViewport(width * 3 / 4, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.0f, 0.0f, 1.0f);
    Ks = vec4(0.7f, 0.6f, 0.6f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- White ---
    glViewport(0, height / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.55f, 0.55f, 0.55f, 1.0f);
    Ks = vec4(0.7f, 0.7f, 0.7f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Yellow Plastic ---
    glViewport(width / 4, height / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.5f, 0.0f, 1.0f);
    Ks = vec4(0.6f, 0.6f, 0.5f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Black ---
    glViewport(width * 2 / 4, height / 6, width / 4, height / 6);

    Ka = vec4(0.02f, 0.02f, 0.02f, 1.0f);
    Kd = vec4(0.01f, 0.01f, 0.01f, 1.0f);
    Ks = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    materialShininess = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Cyan ---
    glViewport(width * 3 / 4, height / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.05f, 0.05f, 1.0f);
    Kd = vec4(0.4f, 0.5f, 0.5f, 1.0f);
    Ks = vec4(0.04f, 0.7f, 0.7f, 1.0f);
    materialShininess   = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Green ---
    glViewport(0, 0, width / 4, height / 6);

    Ka = vec4(0.0f, 0.05f, 0.0f, 1.0f);
    Kd = vec4(0.4f, 0.5f, 0.4f, 1.0f);
    Ks = vec4(0.04f, 0.7f, 0.04f, 1.0f);
    materialShininess   = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Red ---
    glViewport(width / 4, 0, width / 4, height / 6);

    Ka = vec4(0.05f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.4f, 0.4f, 1.0f);
    Ks = vec4(0.7f, 0.04f, 0.04f, 1.0f);
    materialShininess   = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- White ---
    glViewport(width * 2 / 4, 0, width / 4, height / 6);

    Ka = vec4(0.05f, 0.05f, 0.05f, 1.0f);
    Kd = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    Ks = vec4(0.7f, 0.7f, 0.7f, 1.0f);
    materialShininess = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Yellow Rubber ---
    glViewport(width * 3 / 4, 0, width / 4, height / 6);

    Ka = vec4(0.05f, 0.05f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    Ks = vec4(0.7f, 0.7f, 0.04f, 1.0f);
    materialShininess = 0.078125f * 128.0f;
    
    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    glBindVertexArray(0);
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




