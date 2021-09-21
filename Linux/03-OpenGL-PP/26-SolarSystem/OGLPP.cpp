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

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct Stack
{
    mat4 matrix;
    struct Stack* next;
};

//global function declarations
struct Stack* create_stack();
void push_matrix(struct Stack* stack, mat4 curr_matrix);
mat4 pop_matrix(struct Stack* stack);
mat4 peek_matrix(struct Stack* stack);
void delete_stack(struct Stack* stack);

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
char keys[26];

GLuint vertexShaderObject;              //handle to vertex shader object
GLuint fragmentShaderObject;            //handle to fragment shader object
GLuint shaderProgramObject;             //handle to shader program object

GLuint vao_sphere;                      //handle to vertex array object for sphere
GLuint vbo_sphere_position;             //handle to vertex buffer object for vertices of sphere
GLuint vbo_sphere_normal;               //handle to vertex buffer object for normals of sphere
GLuint vbo_sphere_texcoord;             //handle to vertex buffer object for texcoords of sphere
GLuint vbo_sphere_elements;             //handle to vertex buffer object for indices 

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint perspectiveProjectionMatrixUniform;
GLuint materialColorUniform;
               
mat4 perspectiveProjectionMatrix;  
vec4 materialColor;

GLfloat sphere_vertices[1146];
GLfloat sphere_normals[1146];
GLfloat sphere_texcoords[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

int year = 0;
int day = 0;
struct Stack* stack;

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
                    
                    XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
                    switch(keys[0])
                    {
                        case 'D':
                            day = (day + 6) % 360;
                            break;

                        case 'd':
                            day = (day - 6) % 360;
                            break;

                        case 'Y':
                            year = (year + 3) % 360;
                            break;
                        
                        case 'y':
                            year = (year - 3) % 360;
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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Solar System");

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
    
    //--- Vertex Shader ---

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                                                     \
        "\n"                                                                                                    \
        
        "in vec4 vPosition;"                                                                                    \
        
        "uniform mat4 u_modelMatrix;"                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \

        "void main(void)"                                                                                       \
        "{"                                                                                                     \
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

    printf("Vertex Shader Compiled Successfully.\n");

    //--- Fragment Shader ---

    //create shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                       \
        "\n"                                                      \
        
        "uniform vec3 u_materialColor;"                           \
        "out vec4 fragColor;"                                     \
        
        "void main(void)"                                         \
        "{"                                                       \
        "   fragColor = vec4(u_materialColor, 1.0f);"             \
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
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

    //link shader program
    glLinkProgram(shaderProgramObject);
    
    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus > 0)
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
                uninitialize();
                exit(1);
            }
        }
    }
    
    printf("Shader Program Linked Successfully.\n");
    
    //get MVP uniform location
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    perspectiveProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_perspectiveProjectionMatrix");

    materialColorUniform = glGetUniformLocation(shaderProgramObject, "u_materialColor");
    
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

    glGenBuffers(1, &vbo_sphere_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);

    //unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glShadeModel(GL_SMOOTH);
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_CULL_FACE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    perspectiveProjectionMatrix = mat4::identity();
    
    stack = create_stack();
    
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
    mat4 translateMatrix;
    mat4 rotationMatrix;
    mat4 scaleMatrix;
    
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shaderProgramObject);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    //sun
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    viewMatrix = vmath::lookat(vec3(0.0f, 0.0f, 3.5f),
                               vec3(0.0f, 0.0f, 0.0f),
                               vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    glUniform3f(materialColorUniform, 1.0f, 1.0f, 0.0f);

    glBindVertexArray(vao_sphere);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    //earth
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    push_matrix(stack, modelMatrix);

    rotationMatrix = vmath::rotate((float)year, 0.0f, 1.0f, 0.0f);
    translateMatrix = vmath::translate(1.5f, 0.0f, 0.0f);
    scaleMatrix = vmath::scale(0.5f, 0.5f, 0.5f);
    modelMatrix = modelMatrix * rotationMatrix * translateMatrix * scaleMatrix;

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    glUniform3f(materialColorUniform, 0.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    //moon
    push_matrix(stack, modelMatrix);

    rotationMatrix = vmath::rotate((float)day, 0.0f, 1.0f, 0.0f);
    translateMatrix = vmath::translate(1.0f, 0.0f, 0.0f);
    scaleMatrix = vmath::scale(0.5f, 0.5f, 0.5f);

    modelMatrix = modelMatrix * rotationMatrix * translateMatrix * scaleMatrix;

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    pop_matrix(stack);
    pop_matrix(stack);

    glBindVertexArray(0);
    glUseProgram(0);
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void uninitialize(void)
{
    //variable declarations
    GLXContext currentGLXContext;
    
    //code
    if(stack)
    {
        delete_stack(stack);
        stack = NULL;
    }

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

    if(vbo_sphere_elements)
    {
        glDeleteBuffers(1, &vbo_sphere_elements);
        vbo_sphere_elements = 0;
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

struct Stack* create_stack()
{
    //variable declarations
    struct Stack* temp = NULL;

    //code
    temp = (struct Stack*)calloc(1, sizeof(struct Stack));
    if(temp == NULL)
    {
        printf("Error : failed to allocate memory for stack.\n");
        uninitialize();
        exit(1);
    }

    return (temp);
}

void push_matrix(struct Stack* stack, mat4 curr_matrix)
{
    //variable declarations
    struct Stack* temp = NULL;
    struct Stack* curr = NULL;

    //code
    if(stack == NULL)
    {
        stack->matrix = curr_matrix;
        stack->next = NULL;
    }
    else
    {
        curr = (struct Stack*)calloc(1, sizeof(struct Stack));
        if(curr == NULL)
        {
            printf("Error : failed to allocate memory for stack.\n");
            uninitialize();
            exit(1);
        }
        curr->matrix = curr_matrix;
        curr->next = NULL;

        temp = stack;
        while(temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = curr;
    }
}

mat4 pop_matrix(struct Stack* stack)
{
    //variable declarations
    struct Stack* temp = NULL;
    struct Stack* prev = NULL;
    mat4 matrix;

    //code
    if(stack->next == NULL)
    {
        matrix = stack->matrix;
        if(stack)
        {
            free(stack);
        }
        return (matrix);
    }
    else
    {
        temp = stack;
        prev = temp;
        while(temp->next != NULL)
        {
            prev = temp;
            temp = temp->next;
        }
        matrix = temp->matrix;
        
        prev->next = NULL;
        if(temp)
        {
            free(temp);
        }
        return (matrix);
    }

    printf("Error : stack underflow.\n");
    uninitialize();
    exit(1);
}

mat4 peek_matrix(struct Stack* stack)
{
    //variable declarations
    struct Stack* temp = NULL;
    mat4 matrix;

    //code
    temp = stack;
    while(temp->next != NULL)
    {
        temp = temp->next;
    }

    matrix = temp->matrix;
    return (matrix);
}

void delete_stack(struct Stack* stack)
{
    //variable declarations
    struct Stack* temp = NULL;
    struct Stack* curr = NULL;

    //code
    temp = stack;
    while(temp != NULL)
    {
        curr = temp;
        temp = temp->next;

        if(curr)
        {
            free(curr);
        }
    }
}



