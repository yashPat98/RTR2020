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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Deathly Hallows Animation");

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
    //function declaration
    float distance(float x1, float y1, float x2, float y2);

    //variable declaration
    float x, y, fAngle, radius;
    float step = 0.0001f;
   
    static GLfloat Tcx = -5.0f; 
    static GLfloat Tcy = -5.0f;
    
    static GLfloat Twy = 5.0f;

    static GLfloat Tsx = 5.0f;
    static GLfloat Tsy = -5.0f;

    static GLfloat cRot = 0.0f;
    static GLfloat sRot = 0.0f;

    bool cloak = true;
    bool wand = false;
    bool stone = false;

    float lab, lbc, lac, sum;
    float xin, yin, semi;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();    

    glLineWidth(4);

    //invisibility cloak
    if(cloak)
    {
        glTranslatef(Tcx, Tcy, -3.0f);
        glRotatef(cRot, 0.0f, 1.0f, 0.0f);
        glScalef(0.5f, 0.5f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex3f(0.0f, 1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 0.0f);
        glEnd();

        if(Tcx >= 0.0f)
            stone = true;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 

    //resurrection stone
    if(stone)
    {
        lab = distance(0.0f, 1.0f, -1.0f, -1.0f);
        lbc = distance(-1.0f, -1.0f, 1.0f, -1.0f);
        lac = distance(0.0f, 1.0f, 1.0f, -1.0f);
        sum = lab + lbc + lac;

        xin = ((lbc * 0.0f) + (lac * (-1.0f)) + (lab * 1.0f)) / sum;
        yin = ((lbc * 1.0f) + (lac * (-1.0f)) + (lab * (-1.0f))) / sum;

        //translate to incentre
        glTranslatef(Tsx, Tsy, -3.0f);
        glRotatef(sRot, 0.0f, 1.0f, 0.0f);
        glScalef(0.5f, 0.5f, 0.5f);
        
        //radius of incircle = area / semi-perimeter;
        semi = (lab + lbc + lac) / 2;
        radius = sqrt(semi * (semi - lab) * (semi - lbc) * (semi - lac)) / semi;
        
        glBegin(GL_LINE_LOOP);
            for(fAngle = 0.0f; fAngle < 2 * M_PI; fAngle = fAngle + 0.05f)
            {
                x = radius * sin(fAngle);
                y = radius * cos(fAngle);

                glVertex3f(xin + x, yin + y, 0.0f);
            }
        glEnd();

        if(Tsx <= 0.0f)
            wand = true;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 

    //elder wand
    if(wand)
    {
        glTranslatef(0.0f, Twy, -3.0f);
        glScalef(0.5f, 0.5f, 0.5f);

        glBegin(GL_LINES);
            glVertex3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, -1.0f, 0.0f);
        glEnd();
    }

    //Update
    if(cloak)
    {
        if(Tcx < 0.0f)
            Tcx = Tcx + step;

        if(Tcy < 0.0f)
            Tcy = Tcy + step;    
    }

    if(wand)
    {
        if(Twy > 0.0f)
            Twy = Twy - step;
    }

    if(stone)
    {
        if(Tsx > 0.0f)
            Tsx = Tsx - step;

        if(Tsy < 0.0f)
            Tsy = Tsy + step;  
    }

    cRot = cRot + 0.01f;
    if(cRot >= 360.0f)
        cRot = 0.0f;

    sRot = sRot + 0.01f;
    if(sRot >= 360.0f)
        sRot = 0.0f;
    
    glXSwapBuffers(gpDisplay, gWindow);
}

float distance(float x1, float y1, float x2, float y2)
{
    //code
    float result = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
    return ((float)sqrt(result));
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




