//headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "Map.h"

#define MAX_PARTICLES 1000                              //no of particles
#define PI            3.141592f                         //constant
#define MAX_SIZE      379                               //no of pinits of silhouette

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

struct PARTICLE                                           //particle structure 
{
    //state of particle
    bool active;
    float life;
    float fade;
    float rotation;
    float size;

    //position of particle
    float x, y;

    //speed of particle
    float xi, yi;

} particles[MAX_PARTICLES];

bool gbRenderJet = false;                                  //flag to start jet animation
bool gbRotation = true;                                    //flag to start smoke rendering
bool gbHoverPlane = false;                                 //if plane hover on a draw mid 
const float a = 0.008f;                                    //parabola constant y = a * x * x

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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Dynamic India");

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
    
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);

    //scaling silhouette points 
    for(int i = 0; i < MAX_SIZE; i++)
    {
        finalPos[i][0] *= 0.001f;
        finalPos[i][1] *= -0.001f;
    }
    
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
    void DrawIndia(void);
    void DrawJet(int jetNum);
    void DrawSilhouette(void);

    //variable declaration
    static bool bRender = false;
    static int frameCount = 0;

    static GLfloat JetTrans = -83.0f;
    static GLfloat xParabola = -83.0f;
    static GLfloat xRot = -50.0f;
    GLfloat yParabola = 0.0f;
    GLfloat step = 0.00083f;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawSilhouette();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.35f, -4.0f);
    glScalef(0.1f, 0.1f, 0.1f);

    if(bRender)
        DrawIndia();

    if(gbRenderJet)
    {
        
        yParabola = a * xParabola * xParabola;
        
        if(yParabola < 3.0f)
        {
            yParabola = 3.0f;
            gbRotation = false;
        }

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(xParabola + 9.0f, -yParabola, -100.0f);
        glRotatef(-xRot, 0.0f, 0.0f, 1.0f);

        DrawJet(3);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(JetTrans + 9.0f, 0.0f, -100.0f);

        DrawJet(2); 

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(xParabola + 9.0f, yParabola, -100.0f);
        glRotatef(xRot, 0.0f, 0.0f, 1.0f);

        DrawJet(1);
    }

    //update 
    if(bRender == false)
    {
        frameCount++;
        if(frameCount > 380)
            bRender = true;
    }

    if(gbRenderJet)
    {
        if(JetTrans >= 30.0f)
            gbHoverPlane = true;

        if(JetTrans <= 150.0f)
            JetTrans = JetTrans + 0.001f;

        if(gbRotation)
            xRot += step;
        else if(yParabola > 4.1f)
        {
            gbRotation = true;
            step = -step;
        }

        xParabola += 0.001f;
    }
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void DrawIndia(void)
{
    //variable declaration
    static bool IFlag = true;
    static bool nFlag = false;
    static bool dFlag = false;
    static bool iFlag = false;
    static bool aFlag = false;

    static GLfloat ITrans = -32.0f;
    static GLfloat nTrans = 22.0f;
    static GLfloat dCol   = 0.0f;
    static GLfloat iTrans = -20.0f;
    static GLfloat aTrans = 30.0f;
    float speed = 0.0003f;

    GLfloat x, y;
    GLfloat radius = 4.5f;
    GLfloat r = 0.3125f;
    GLfloat g = 0.61718f;
    GLfloat b = 0.18359f;

    //code
    //I
    if(IFlag)
    {
        glTranslatef(ITrans, 0.0f, 0.0f);
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
    }

    //N
    if(nFlag)
    {
        glTranslatef(3.0f, nTrans, 0.0f);
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
    }

    //D
    if(dFlag)
    {
        glTranslatef(3.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
            //line
            glColor4f(1.0f, 0.55859f, 0.10937f, dCol);
            glVertex3f(4.5f, 1.0f, 0.0f);
            glVertex3f(5.0f, 0.0f, 0.0f);
            glVertex3f(6.5f, 0.0f, 0.0f);
            glVertex3f(6.5f, 1.0, 0.0f);

            glVertex3f(6.5f, 0.0f, 0.0f);
            glVertex3f(5.5f, 0.0f, 0.0f);
            glColor4f(0.3125f, 0.61718f, 0.18359f, dCol);
            glVertex3f(5.5f, -7.0f, 0.0f);
            glVertex3f(6.5f, -8.0f, 0.0f);
        glEnd();

        glTranslatef(6.5f, -3.5f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
            glColor4f(0.3125f, 0.61718f, 0.18359f, dCol);
            glVertex3f(0.0f, 0.0f, 0.0f);
            for(float angle = 0.0f; angle <= PI; angle += (PI / 17.0f))
            {
                x = radius * sin(angle);
                y = -radius * cos(angle);
        
                glColor4f(r, g, b, dCol);
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
            for(float angle = 0.0f; angle <= PI; angle += (PI / 17.0f))
            {
                x = radius * sin(angle);
                y = radius * cos(angle);

                glVertex3f(x, y, 0.0f);
            }
        glEnd();
    }

    //I
    if(iFlag)
    {
        glTranslatef(6.0f, iTrans, 0.0f);
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
    }

    //A
    if(aFlag)
    {
        glTranslatef(aTrans, 0.0f, 0.0f);
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
            if(gbHoverPlane)
            {
                glColor3f(1.0f, 0.55859f, 0.10937f);
                glVertex3f(-1.2f, -3.0f, 0.0f);
                glColor3f(0.3125f, 0.61718f, 0.18359f);
                glVertex3f(-0.7f, -4.0f, 0.0f);
                glVertex3f(3.2f, -4.0f, 0.0f);
                glColor3f(1.0f, 0.55859f, 0.10937f);
                glVertex3f(2.7f, -3.0f, 0.0f);
            }
        glEnd();
    }

    //update
    if(IFlag)
    {
        if(ITrans < -14.0f)
            ITrans += speed;
        else
            nFlag = true;
    }

    if(nFlag)
    {
        if(nTrans > 0.0f)
            nTrans -= (speed + 0.0003f);
        else
            dFlag = true;
    }

    if(dFlag)
    {
        if(dCol < 1.0f)
            dCol += 0.001f;
        
        if(dCol > 0.7f)
            iFlag = true;
    }

    if(iFlag)
    {
        if(iTrans < 3.5f)
            iTrans += speed;
        else
            aFlag = true;
    }

    if(aFlag)
    {
        if(aTrans > 5.0f)
            aTrans -= speed;

        if(aTrans < 6.0f)
            gbRenderJet = true;
    }
}

void DrawJet(int jetNum)
{
    //variable declaration
    float radius = 0.5f;

    //code
    //top
    glBegin(GL_TRIANGLES);
        glColor3f(0.18359375f, 0.23828125f, 0.2890625f);
        glVertex3f(6.5f, 0.0f, 0.0f);
        glVertex3f(4.0f, 0.8f, 0.0f);
        glVertex3f(4.0f, -0.8f, 0.0f);

        glColor3f(0.23828125f, 0.2890625f, 0.35546875f);
        glVertex3f(5.7f, 0.0f, 0.0f);
        glVertex3f(4.0f, 0.5f, 0.0f);
        glVertex3f(4.0f, -0.5f, 0.0f);
    glEnd();

    //glass canopy front
    glColor3f(0.18359375f, 0.23828125f, 0.2890625f);
    glBegin(GL_QUADS);
        glVertex3f(4.0f, 0.8f, 0.0f);
        glVertex3f(0.5f, 1.3f, 0.0f);
        glVertex3f(0.5f, -1.3f, 0.0f);
        glVertex3f(4.0f, -0.8f, 0.0f);
    glEnd();

    //glass canopy back
    glBegin(GL_QUADS);
        glVertex3f(0.5f, 1.3f, 0.0f);
        glVertex3f(-3.0f, 1.3f, 0.0f);
        glVertex3f(-3.0f, -1.3f, 0.0f);
        glVertex3f(0.5f, -1.3f, 0.0f);
    glEnd();

    //top mini wing
    glBegin(GL_QUADS);
        glColor3f(0.67578125f, 0.84375f, 0.8984375f);
        glVertex3f(0.5f, 1.3f, 0.0f);
        glVertex3f(-1.1f, 3.0f, 0.0f);
        glVertex3f(-1.6f, 3.0f, 0.0f);
        glVertex3f(-1.6f, 1.3f, 0.0f);
    glEnd();

    //bottom mini wing
    glBegin(GL_QUADS);
        glVertex3f(0.5f, -1.3f, 0.0f);
        glVertex3f(-1.1f, -3.0f, 0.0f);
        glVertex3f(-1.6f, -3.0f, 0.0f);
        glVertex3f(-1.6f, -1.3f, 0.0f);
    glEnd();

    //mid 
    glBegin(GL_QUADS);
        glColor3f(0.18359375f, 0.23828125f, 0.2890625f); 
        glVertex3f(-3.0f, 1.3f, 0.0f);
        glVertex3f(-6.5f, 1.3f, 0.0f);
        glVertex3f(-6.5f, -1.3f, 0.0f);
        glVertex3f(-3.0f, -1.3f, 0.0f);

        glColor3f(0.078125f, 0.12890625f, 0.19140625f);
        glVertex3f(-5.5f, 0.8f, 0.0f);
        glVertex3f(-5.5f, 0.3f, 0.0f);
        glVertex3f(-0.5f, 0.3f, 0.0f);
        glVertex3f(-0.5f, 0.8f, 0.0f);

        glVertex3f(-5.5f, -0.8f, 0.0f);
        glVertex3f(-5.5f, -0.3f, 0.0f);
        glVertex3f(-0.5f, -0.3f, 0.0f);
        glVertex3f(-0.5f, -0.8f, 0.0f);
    glEnd();

    glBegin(GL_TRIANGLES);     
        glVertex3f(-0.5f, 0.8f, 0.0f);
        glVertex3f(-0.5f, 0.3f, 0.0f);
        glVertex3f(0.5f, 0.5f, 0.0f);

        glVertex3f(-0.5f, -0.8f, 0.0f);
        glVertex3f(-0.5f, -0.3f, 0.0f);
        glVertex3f(0.5f, -0.5f, 0.0f);
    glEnd();

    //top wing
    glBegin(GL_QUADS);
        glColor3f(0.67578125f, 0.84375f, 0.8984375f);
        glVertex3f(-1.6f, 1.3f, 0.0f);
        glVertex3f(-6.0f, 6.5f, 0.0f);
        glVertex3f(-7.5f, 6.5f, 0.0f);
        glVertex3f(-6.5f, 1.3f, 0.0f);

        glColor3f(0.171875f, 0.23046875f, 0.3046875f);
        glVertex3f(-1.6f, 1.3f, 0.0f);
        glVertex3f(-5.1f, 5.5f, 0.0f);
        glVertex3f(-5.6f, 5.5f, 0.0f);
        glVertex3f(-2.4f, 1.3f, 0.0f);

        glVertex3f(-7.1f, 6.5f, 0.0f);
        glVertex3f(-7.5f, 6.5f, 0.0f);
        glVertex3f(-6.5f, 1.3f, 0.0f);
        glVertex3f(-5.8f, 1.3f, 0.0f);
    glEnd();

    //bottom wing
    glBegin(GL_QUADS);
        glColor3f(0.67578125f, 0.84375f, 0.8984375f);
        glVertex3f(-1.6f, -1.3f, 0.0f);
        glVertex3f(-6.0f, -6.5f, 0.0f);
        glVertex3f(-7.5f, -6.5f, 0.0f);
        glVertex3f(-6.5f, -1.3f, 0.0f);

        glColor3f(0.171875f, 0.23046875f, 0.3046875f);
        glVertex3f(-1.6f, -1.3f, 0.0f);
        glVertex3f(-5.1f, -5.5f, 0.0f);
        glVertex3f(-5.6f, -5.5f, 0.0f);
        glVertex3f(-2.4f, -1.3f, 0.0f);

        glVertex3f(-7.1f, -6.5f, 0.0f);
        glVertex3f(-7.5f, -6.5f, 0.0f);
        glVertex3f(-6.5f, -1.3f, 0.0f);
        glVertex3f(-5.8f, -1.3f, 0.0f);
    glEnd();

    //tail 
    glColor3f(0.078125f, 0.12890625f, 0.19140625f);
    glBegin(GL_QUADS);
        glVertex3f(-6.5f, 0.8f, 0.0f);
        glVertex3f(-10.0f, 0.8f, 0.0f);
        glVertex3f(-10.0f, -0.8f, 0.0f);
        glVertex3f(-6.5f, -0.8f, 0.0f);
    glEnd();

    //top side
    glBegin(GL_POLYGON);
        glColor3f(0.171875f, 0.23046875f, 0.3046875f);
        glVertex3f(-6.5f, 1.3f, 0.0f);
        glVertex3f(-10.5f, 1.3f, 0.0f);
        glVertex3f(-10.5f, 1.0f, 0.0f);
        glVertex3f(-10.0f, 0.8f, 0.0f);
        glVertex3f(-6.5f, 0.8f, 0.0f);
    glEnd();

    //bottom side
    glBegin(GL_POLYGON);
        glVertex3f(-6.5f, -1.3f, 0.0f);
        glVertex3f(-10.5f, -1.3f, 0.0f);
        glVertex3f(-10.5f, -1.0f, 0.0f);
        glVertex3f(-10.0f, -0.8f, 0.0f);
        glVertex3f(-6.5f, -0.8f, 0.0f);
    glEnd();

    //top tail wing
    glBegin(GL_QUADS);
        glColor3f(0.67578125f, 0.84375f, 0.8984375f);
        glVertex3f(-6.5f, 1.3f, 0.0f);
        glVertex3f(-8.7f, 3.5f, 0.0f);
        glVertex3f(-10.5f, 2.5f, 0.0f);
        glVertex3f(-10.0f, 1.3f, 0.0f);
    glEnd();

    //bottom tail wing
    glBegin(GL_QUADS);
        glVertex3f(-6.5f, -1.3f, 0.0f);
        glVertex3f(-8.7f, -3.5f, 0.0f);
        glVertex3f(-10.5f, -2.5f, 0.0f);
        glVertex3f(-10.0f, -1.3f, 0.0f);
    glEnd();

    //end
    glBegin(GL_QUADS);
        glColor3f(0.171875f, 0.23046875f, 0.3046875f);
        glVertex3f(-10.0f, 0.8f, 0.0f);
        glVertex3f(-11.5f, 0.5f, 0.0f);
        glVertex3f(-11.5f, -0.5f, 0.0f);
        glVertex3f(-10.0f, -0.8f, 0.0f);
    glEnd();

    //roundel
    //outer circle
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-6.0f, 5.0f, 0.0f);
        for(float angle = 0.0f; angle <= 2 * PI; angle += (PI / 9.0f))
        {
            glVertex3f(-6.0f + radius * (float)sin(angle), 5.0f + radius * (float)cos(angle), 0.0f);
        }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1.0f, 0.55859f, 0.10937f);
        glVertex3f(-6.0f, -5.0f, 0.0f);
        for(float angle = 0.0f; angle <= 2 * PI; angle += (PI / 9.0f))
        {
            glVertex3f(-6.0f + radius * (float)sin(angle), -5.0f + radius * (float)cos(angle), 0.0f);
        }
    glEnd();

    //central circle
    radius = 0.3f;
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-6.0f, 5.0f, 0.0f);
        for(float angle = 0.0f; angle <= 2 * PI; angle += (PI / 9.0f))
        {
            glVertex3f(-6.0f + radius * (float)sin(angle), 5.0f + radius * (float)cos(angle), 0.0f);
        }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-6.0f, -5.0f, 0.0f);
        for(float angle = 0.0f; angle <= 2 * PI; angle += (PI / 9.0f))
        {
            glVertex3f(-6.0f + radius * (float)sin(angle), -5.0f + radius * (float)cos(angle), 0.0f);
        }
    glEnd();

    //inner circle
    radius = 0.1f;
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(-6.0f, 5.0f, 0.0f);
        for(float angle = 0.0f; angle <= 2 * PI; angle += (PI / 9.0f))
        {
            glVertex3f(-6.0f + radius * (float)sin(angle), 5.0f + radius * (float)cos(angle), 0.0f);
        }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.3125f, 0.61718f, 0.18359f);
        glVertex3f(-6.0f, -5.0f, 0.0f);
        for(float angle = 0.0f; angle <= 2 * PI; angle += (PI / 9.0f))
        {
            glVertex3f(-6.0f + radius * (float)sin(angle), -5.0f + radius * (float)cos(angle), 0.0f);
        }
    glEnd();

    //I
    glTranslatef(1.0f, 0.5f, 0.0f);
    glBegin(GL_QUADS);
        //line
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-0.05f, 0.1f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.1f, 0.1f, 0.0f);

        glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.05f, 0.0f, 0.0f);
        glVertex3f(0.05f, -0.7f, 0.0f);
        glVertex3f(0.15f, -0.8f, 0.0f);
    glEnd();

    //A
    glTranslatef(0.45f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        //left side
        glVertex3f(-0.05f, 0.1f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.1f, 0.1f, 0.0f);

        glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.05f, 0.0f, 0.0f);
        glVertex3f(-0.15f, -0.7f, 0.0f);
        glVertex3f(-0.05f, -0.8f, 0.0f);

        //right side
        glVertex3f(0.1f, 0.1f, 0.0f);
        glVertex3f(0.1f, 0.0f, 0.0f);
        glVertex3f(0.48f, -0.7f, 0.0f);
        glVertex3f(0.6f, -0.8f, 0.0f);

        //mid
        glVertex3f(-0.12f, -0.3f, 0.0f);
        glVertex3f(-0.07f, -0.4f, 0.0f);
        glVertex3f(0.32f, -0.4f, 0.0f);
        glVertex3f(0.27f, -0.3f, 0.0f);
    glEnd();

    glTranslatef(0.65f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex3f(-0.05f, 0.1f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.45f, 0.0f, 0.0f);
        glVertex3f(0.40f, 0.1f, 0.0f);

        glVertex3f(0.15f, 0.0f, 0.0f);
        glVertex3f(0.05f, 0.0f, 0.0f);
        glVertex3f(0.05f, -0.7f, 0.0f);
        glVertex3f(0.15f, -0.8f, 0.0f);

        glVertex3f(0.05f, -0.3f, 0.0f);
        glVertex3f(0.05f, -0.4f, 0.0f);
        glVertex3f(0.35f, -0.4f, 0.0f);
        glVertex3f(0.31f, -0.3f, 0.0f);
    glEnd();
}

void DrawSilhouette(void)
{
    //variable declaration
    static int renderPoints = 0;
    static int count = 0;
    static GLenum ePrimitive = GL_POINTS;

    //code
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glTranslatef(-0.5f, 0.53f, -1.4f);
    glPointSize(2.0f);

    glBegin(ePrimitive);
        int temp = renderPoints;

        glColor3f(1.0f, 1.0f, 1.0f);
        for(int i = 0; (i < MAX_SIZE) && (temp != 0); i++, temp--)
        {
            glVertex3f(finalPos[i][0], finalPos[i][1], 0.0f);
        }
    glEnd();

    //update
    if(count != 1000)
        count++;
    else
    {
        renderPoints++;
        count = 0;
    }

    if(renderPoints == 379)
        ePrimitive = GL_LINE_LOOP;

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




