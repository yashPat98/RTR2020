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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Static India");

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
    GLfloat x, y;
    GLfloat radius = 4.5f;
    GLfloat r = 0.3125f;
    GLfloat g = 0.61718f;
    GLfloat b = 0.18359f;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.35f, -3.5f);
    glScalef(0.1f, 0.1f, 0.1f);

    //I
    glTranslatef(-14.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        //line
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-0.5f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0, 0.0f);

        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(0.5f, -7.0f, 0.0f);
        glVertex3f(1.5f, -8.0f, 0.0f);
    glEnd();

    //N
    glTranslatef(3.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        //line
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-0.5f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0, 0.0f);

        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(0.5f, -7.0f, 0.0f);
        glVertex3f(1.5f, -8.0f, 0.0f);

        //mid
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(1.0f, 1.0, 0.0f);
        glVertex3f(1.5f, -0.5f, 0.0f);
        glColor3f(0.6562f, 0.58788f, 0.14648f);
        glVertex3f(5.5f, -5.3f, 0.0f);
        glVertex3f(5.5f, -4.3f, 0.0f); 

        //line
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(4.5f, 1.0f, 0.0f);
        glVertex3f(5.0f, 0.0f, 0.0f);
        glVertex3f(6.5f, 0.0f, 0.0f);
        glVertex3f(6.0f, 1.0, 0.0f);

        glVertex3f(6.5f, 0.0f, 0.0f);
        glVertex3f(5.5f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(5.5f, -7.0f, 0.0f);
        glVertex3f(6.5f, -8.0f, 0.0f);
    glEnd();

    //D
    glTranslatef(3.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        //line
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(4.5f, 1.0f, 0.0f);
        glVertex3f(5.0f, 0.0f, 0.0f);
        glVertex3f(6.5f, 0.0f, 0.0f);
        glVertex3f(6.5f, 1.0, 0.0f);

        glVertex3f(6.5f, 0.0f, 0.0f);
        glVertex3f(5.5f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(5.5f, -7.0f, 0.0f);
        glVertex3f(6.5f, -8.0f, 0.0f);
    glEnd();

    glTranslatef(6.5f, -3.5f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for(float angle = 0.0f; angle <= M_PI; angle += (M_PI / 18.0f))
        {
            x = radius * sin(angle);
            y = -radius * cos(angle);
        
            glColor3f(r, g, b);
            glVertex3f(x, y, 0.0f);

            r += (1.0f - 0.3125f) / 17.0f;
            g -= (0.61718f - 0.55859f) / 17.0f;
            b -= (0.18359f - 0.10937f) / 17.0f;
        }
    glEnd();

    radius = 3.6f;
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for(float angle = 0.0f; angle <= M_PI; angle += (M_PI / 18.0f))
        {
            x = radius * sin(angle);
            y = radius * cos(angle);

            glVertex3f(x, y, 0.0f);
        }
    glEnd();

    //I
    glTranslatef(6.0f, 3.5f, 0.0f);
    glBegin(GL_QUADS);
        //line
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-0.5f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0, 0.0f);

        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(0.5f, -7.0f, 0.0f);
        glVertex3f(1.5f, -8.0f, 0.0f);
    glEnd();

    //A
    glTranslatef(5.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        //left side
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-0.5f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0, 0.0f);

        glVertex3f(1.5f, 0.0f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(-1.5f, -7.0f, 0.0f);
        glVertex3f(-0.5f, -8.0f, 0.0f);

        //right side
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(1.0f, 1.0, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(4.8f, -7.0f, 0.0f);
        glVertex3f(6.0f, -8.0f, 0.0f);

        //mid
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-1.2f, -3.0f, 0.0f);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(-0.7f, -4.0f, 0.0f);
        glVertex3f(3.2f, -4.0f, 0.0f);
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(2.7f, -3.0f, 0.0f);
    glEnd();
    
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




