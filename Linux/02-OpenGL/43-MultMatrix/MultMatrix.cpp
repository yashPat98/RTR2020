//headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#define USE_MATH_DEFINES
#include <math.h>

//namespaces
using namespace std;

//global variable declarations
Display      *gpDisplay     = NULL;
XVisualInfo  *gpXVisualInfo = NULL;
Colormap      gColormap;
Window        gWindow;
GLXContext    gGLXContext;

int giWindowHeight = 600;
int giWindowWidth  = 800;
bool gbFullscreen  = false;

//column major matrices
GLfloat identityMatrix[16];                              
GLfloat translationMatrix[16];                           
GLfloat scaleMatrix[16];                                
GLfloat rotationMatrix_X[16];                            
GLfloat rotationMatrix_Y[16];                            
GLfloat rotationMatrix_Z[16];                          

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
    int defaultScreen;
    int styleMask;
    
    static int frameBufferAttributes[] = { GLX_DOUBLEBUFFER, True,
                                           GLX_RGBA,
                                           GLX_RED_SIZE,        8,
                                           GLX_GREEN_SIZE,      8,
                                           GLX_BLUE_SIZE,       8,
                                           GLX_ALPHA_SIZE,      8,
                                           GLX_DEPTH_SIZE,     24,
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
    
    gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR : Unable To Get A Viusal.\nExiting Now...\n");
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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : MultMatrix");

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
    
    //code
    gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
    glXMakeCurrent(gpDisplay, gWindow, gGLXContext);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    //inline initialization of identity matrix
    identityMatrix[0] = 1.0f;
    identityMatrix[1] = 0.0f;
    identityMatrix[2] = 0.0f;
    identityMatrix[3] = 0.0f;

    identityMatrix[4] = 0.0f;
    identityMatrix[5] = 1.0f;
    identityMatrix[6] = 0.0f;
    identityMatrix[7] = 0.0f;

    identityMatrix[8] = 0.0f;
    identityMatrix[9] = 0.0f;
    identityMatrix[10] = 1.0f;
    identityMatrix[11] = 0.0f;

    identityMatrix[12] = 0.0f;
    identityMatrix[13] = 0.0f;
    identityMatrix[14] = 0.0f;
    identityMatrix[15] = 1.0f;

    //inline initialization of translation matrix
    translationMatrix[0] = 1.0f;
    translationMatrix[1] = 0.0f;
    translationMatrix[2] = 0.0f;
    translationMatrix[3] = 0.0f;

    translationMatrix[4] = 0.0f;
    translationMatrix[5] = 1.0f;
    translationMatrix[6] = 0.0f;
    translationMatrix[7] = 0.0f;

    translationMatrix[8] = 0.0f;
    translationMatrix[9] = 0.0f;
    translationMatrix[10] = 1.0f;
    translationMatrix[11] = 0.0f;

    translationMatrix[12] = 0.0f;                   
    translationMatrix[13] = 0.0f;                   
    translationMatrix[14] = -6.0f;                  
    translationMatrix[15] = 1.0f;
  
    //inline initialization of scale matrix
    scaleMatrix[0] = 0.75f;
    scaleMatrix[1] = 0.0f;
    scaleMatrix[2] = 0.0f;
    scaleMatrix[3] = 0.0f;

    scaleMatrix[4] = 0.0f;
    scaleMatrix[5] = 0.75f;
    scaleMatrix[6] = 0.0f;
    scaleMatrix[7] = 0.0f;

    scaleMatrix[8] = 0.0f;
    scaleMatrix[9] = 0.0f;
    scaleMatrix[10] = 0.75f;
    scaleMatrix[11] = 0.0f;

    scaleMatrix[12] = 0.0f;
    scaleMatrix[13] = 0.0f;
    scaleMatrix[14] = 0.0f;
    scaleMatrix[15] = 1.0f;
    
    Resize(giWindowWidth, giWindowHeight);
}

void Resize(int width, int height)
{
    //code
    if(height == 0)
        height = 1;
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void Render(void)
{
    //variable declaration
    static GLfloat cubeRot = 0.0f;
    static GLfloat cubeRot_rad = 0.0f;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    glLoadMatrixf(identityMatrix);

    //glTranslatef(0.0f,  0.0f, -6.0f);
    glMultMatrixf(translationMatrix);
    //glScalef(0.75f, 0.75f, 0.75f);
    glMultMatrixf(scaleMatrix);

    cubeRot_rad = cubeRot * (M_PI / 180.0f);

    //inline initialization of X-axis rotation matrix
    rotationMatrix_X[0] = 1.0f;
    rotationMatrix_X[1] = 0.0f;
    rotationMatrix_X[2] = 0.0f;
    rotationMatrix_X[3] = 0.0f;

    rotationMatrix_X[4] = 0.0f;
    rotationMatrix_X[5] = cos(cubeRot_rad);
    rotationMatrix_X[6] = sin(cubeRot_rad);
    rotationMatrix_X[7] = 0.0f;

    rotationMatrix_X[8] = 0.0f;
    rotationMatrix_X[9] = -sin(cubeRot_rad);
    rotationMatrix_X[10] = cos(cubeRot_rad);
    rotationMatrix_X[11] = 0.0f;

    rotationMatrix_X[12] = 0.0f;
    rotationMatrix_X[13] = 0.0f;
    rotationMatrix_X[14] = 0.0f;
    rotationMatrix_X[15] = 1.0f;

    //inline initialization of Y-axis rotation matrix
    rotationMatrix_Y[0] = cos(cubeRot_rad);
    rotationMatrix_Y[1] = 0.0f;
    rotationMatrix_Y[2] = -sin(cubeRot_rad);
    rotationMatrix_Y[3] = 0.0f;

    rotationMatrix_Y[4] = 0.0f;
    rotationMatrix_Y[5] = 1.0f;
    rotationMatrix_Y[6] = 0.0f;
    rotationMatrix_Y[7] = 0.0f;

    rotationMatrix_Y[8] = sin(cubeRot_rad);
    rotationMatrix_Y[9] = 0.0f;
    rotationMatrix_Y[10] = cos(cubeRot_rad);
    rotationMatrix_Y[11] = 0.0f;

    rotationMatrix_Y[12] = 0.0f;
    rotationMatrix_Y[13] = 0.0f;
    rotationMatrix_Y[14] = 0.0f;
    rotationMatrix_Y[15] = 1.0f;

    //inline initialization of Z-axis rotation matrix
    rotationMatrix_Z[0] = cos(cubeRot_rad);
    rotationMatrix_Z[1] = sin(cubeRot_rad);
    rotationMatrix_Z[2] = 0.0f;
    rotationMatrix_Z[3] = 0.0f;

    rotationMatrix_Z[4] = -sin(cubeRot_rad);
    rotationMatrix_Z[5] = cos(cubeRot_rad);
    rotationMatrix_Z[6] = 0.0f;
    rotationMatrix_Z[7] = 0.0f;

    rotationMatrix_Z[8] = 0.0f;
    rotationMatrix_Z[9] = 0.0f;
    rotationMatrix_Z[10] = 1.0f;
    rotationMatrix_Z[11] = 0.0f;
    
    rotationMatrix_Z[12] = 0.0f;
    rotationMatrix_Z[13] = 0.0f;
    rotationMatrix_Z[14] = 0.0f;
    rotationMatrix_Z[15] = 1.0f;

    //glRotatef(cubeRot, 1.0f, 0.0f, 0.0f);
    //glRotatef(cubeRot, 0.0f, 1.0f, 0.0f);
    //glRotatef(cubeRot, 0.0f, 0.0f, 1.0f);
    glMultMatrixf(rotationMatrix_X);
    glMultMatrixf(rotationMatrix_Y);
    glMultMatrixf(rotationMatrix_Z);

    glBegin(GL_QUADS);
        //near
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        //right
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        //far
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

        //left
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        //top
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);

        //bottom
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);        
    glEnd();

    //update
    cubeRot = cubeRot + 0.01f;
    if(cubeRot >= 360.0f)
        cubeRot = 0.0f;
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void uninitialize(void)
{
    //variable declarations
    GLXContext currentGLXContext;
    
    //code
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




