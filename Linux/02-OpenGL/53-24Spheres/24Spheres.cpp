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

bool gbLight          = false;
int key_pressed         = 0;

GLUquadric *quadric[24];

GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 3.0f, 3.0f, 0.0f};

GLfloat light_model_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat light_model_local_viewer[] = {0.0f};

GLfloat angle_for_x_rotation = 0.0f;
GLfloat angle_for_y_rotation = 0.0f;
GLfloat angle_for_z_rotation = 0.0f;

int width, height;

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
                            if(gbLight == false)
                            {
                                glEnable(GL_LIGHTING);
                                gbLight = true;
                            }
                            else
                            {
                                glDisable(GL_LIGHTING);
                                gbLight = false;
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
    
    XStoreName(gpDisplay, gWindow, "OpenGL : 24 Spheres");

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
    
    //enable automatic normal calculation
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    //set up lighting model
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer);

    //set up light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);

    //initialize quadric objects
    for(int i = 0; i < 24; i++)
    {
        quadric[i] = gluNewQuadric();
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
    
    if(width <= height)
    {    
        glOrtho( 0.0f,                                          //left
                 15.5f,                                         //right
                 0.0f,                                          //bottom
                 15.5f * ((GLfloat)height / (GLfloat)width),    //top
                -10.0f,                                         //near
                 10.0f);                                        //far
    }
    else
    {
        glOrtho( 0.0f,                                           //left
                 15.5f * ((GLfloat)width / (GLfloat)height),     //right
                 0.0f,                                           //bottom    
                 15.5f,                                          //top
                -10.0f,                                          //near
                 10.0f);                                         //far
    }
}

void Render(void)
{
    //function declaration
    void DrawSpheres(void);

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch(key_pressed)
    {
        case 1:
            glRotatef(angle_for_x_rotation, 1.0f, 0.0f, 0.0f);
            lightPosition[1] = angle_for_x_rotation;
            break;
        
        case 2:
            glRotatef(angle_for_y_rotation, 0.0f, 1.0f, 0.0f);
            lightPosition[0] = angle_for_y_rotation;
            break;
        
        case 3:
            glRotatef(angle_for_z_rotation, 0.0f, 0.0f, 1.0f);
            lightPosition[0] = angle_for_z_rotation;
            break;
        
        default:
            break;
    }

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    DrawSpheres();

    //update
    if(key_pressed == 1)
    {
        angle_for_x_rotation += 0.1f;
    }
    else if(key_pressed == 2)
    {
        angle_for_y_rotation += 0.1f;
    }
    else if(key_pressed == 3)
    {
        angle_for_z_rotation += 0.1f;
    }
    
    glXSwapBuffers(gpDisplay, gWindow);
}

void DrawSpheres(void)
{
    //variable declaration
    GLfloat materialAmbient[4];
    GLfloat materialDiffuse[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;
    GLfloat xCentre, yCentre;

    //code
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Resize(width / 4, height / 6);
    
    if(width <= height)
    {
        xCentre = 15.5f / 2.0f;
        yCentre = (15.5f * (GLfloat)height * 4.0f / (GLfloat)(width * 6.0f)) / 2.0f;
    }
    else
    {
        xCentre = (15.5f * (GLfloat)width * 6.0f / (GLfloat)(height * 4.0f)) / 2.0f;
        yCentre = 15.5f / 2.0f;
    }
    
    glViewport(0, height * 5 / 6, width / 4, height / 6);

    // --- Emerald ---
    materialAmbient[0] = 0.0215f;
    materialAmbient[1] = 0.1745f;
    materialAmbient[2] = 0.0215f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.07568f;
    materialDiffuse[1] = 0.61424f;
    materialDiffuse[2] = 0.07568f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.633f;
    materialSpecular[1] = 0.727811f;
    materialSpecular[2] = 0.633f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.6f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[0], 5.0f, 30, 30);

    glViewport(width / 4, height * 5 / 6, width / 4, height / 6);

    // --- Jade ---
    materialAmbient[0] = 0.135f;
    materialAmbient[1] = 0.2225f;
    materialAmbient[2] = 0.1575f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.54f;
    materialDiffuse[1] = 0.89f;
    materialDiffuse[2] = 0.63f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.316228f;
    materialSpecular[1] = 0.316228f;
    materialSpecular[2] = 0.316228f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.1f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[1], 5.0f, 30, 30);

    glViewport(width * 2 / 4, height * 5 / 6, width / 4, height / 6);

    // --- Obsidian ---
    materialAmbient[0] = 0.05375f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.06625f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.18275f;
    materialDiffuse[1] = 0.17f;
    materialDiffuse[2] = 0.22525f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.332741f;
    materialSpecular[1] = 0.328634f;
    materialSpecular[2] = 0.346435f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.3f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[2], 5.0f, 30, 30);

    glViewport(width * 3 / 4, height * 5 / 6, width / 4, height / 6);

    // --- Pearl ---
    materialAmbient[0] = 0.25f;
    materialAmbient[1] = 0.20725f;
    materialAmbient[2] = 0.20725f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 1.0f;
    materialDiffuse[1] = 0.829f;
    materialDiffuse[2] = 0.829f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.296648f;
    materialSpecular[1] = 0.296648f;
    materialSpecular[2] = 0.296648f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.088f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[3], 5.0f, 30, 30);

    glViewport(0, height * 4 / 6, width / 4, height / 6);

    // --- Ruby ---
    materialAmbient[0] = 0.1745f;
    materialAmbient[1] = 0.01175f;
    materialAmbient[2] = 0.01175f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.61424f;
    materialDiffuse[1] = 0.04136f;
    materialDiffuse[2] = 0.04136f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.727811f;
    materialSpecular[1] = 0.626959f;
    materialSpecular[2] = 0.626959f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.6f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[4], 5.0f, 30, 30);
    
    glViewport(width / 4, height * 4 / 6, width / 4, height / 6);

    // --- Turquoise ---
    materialAmbient[0] = 0.1f;
    materialAmbient[1] = 0.18725f;
    materialAmbient[2] = 0.1745f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.396f;
    materialDiffuse[1] = 0.74151f;
    materialDiffuse[2] = 0.69102f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.297254f;
    materialSpecular[1] = 0.30829f;
    materialSpecular[2] = 0.306678f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.1f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[5], 5.0f, 30, 30);
    
    glViewport(width * 2 / 4, height * 4 / 6, width / 4, height / 6);

    // --- Brass ---
    materialAmbient[0] = 0.329412f;
    materialAmbient[1] = 0.223529f;
    materialAmbient[2] = 0.027451f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.780392f;
    materialDiffuse[1] = 0.568627f;
    materialDiffuse[2] = 0.113725f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.992157f;
    materialSpecular[1] = 0.941176f;
    materialSpecular[2] = 0.807843f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.21794872f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[6], 5.0f, 30, 30);

    glViewport(width * 3 / 4, height * 4 / 6, width / 4, height / 6);

    // --- Bronze ---
    materialAmbient[0] = 0.2125f;
    materialAmbient[1] = 0.1275f;
    materialAmbient[2] = 0.054f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.714f;
    materialDiffuse[1] = 0.4284f;
    materialDiffuse[2] = 0.18144f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.393548f;
    materialSpecular[1] = 0.271906f;
    materialSpecular[2] = 0.166721f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.2f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[7], 5.0f, 30, 30);

    glViewport(0, height * 3 / 6, width / 4, height / 6);

    // --- Chrome ---
    materialAmbient[0] = 0.25f;
    materialAmbient[1] = 0.25f;
    materialAmbient[2] = 0.25f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.4f;
    materialDiffuse[1] = 0.4f;
    materialDiffuse[2] = 0.4f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.774597f;
    materialSpecular[1] = 0.774597f;
    materialSpecular[2] = 0.774597f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.6f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[8], 5.0f, 30, 30);

    glViewport(width / 4, height * 3 / 6, width / 4, height / 6);

    // --- Copper ---
    materialAmbient[0] = 0.19125f;
    materialAmbient[1] = 0.0735f;
    materialAmbient[2] = 0.0225f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.7038f;
    materialDiffuse[1] = 0.27048f;
    materialDiffuse[2] = 0.0828f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.256777f;
    materialSpecular[1] = 0.137622f;
    materialSpecular[2] = 0.086014f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.1f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[9], 5.0f, 30, 30);

    glViewport(width * 2 / 4, height * 3 / 6, width / 4, height / 6);

    // --- Gold ---
    materialAmbient[0] = 0.24725f;
    materialAmbient[1] = 0.1995f;
    materialAmbient[2] = 0.0745f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.75164f;
    materialDiffuse[1] = 0.60648f;
    materialDiffuse[2] = 0.22648f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.628281f;
    materialSpecular[1] = 0.555802f;
    materialSpecular[2] = 0.366065f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.4f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[10], 5.0f, 30, 30);

    glViewport(width * 3 / 4, height * 3 / 6, width / 4, height / 6);

    // --- Silver ---
    materialAmbient[0] = 0.19225f;
    materialAmbient[1] = 0.19225f;
    materialAmbient[2] = 0.19225f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.50754f;
    materialDiffuse[1] = 0.50754f;
    materialDiffuse[2] = 0.50754f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.508273f;
    materialSpecular[1] = 0.508273f;
    materialSpecular[2] = 0.508273f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.4f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[11], 5.0f, 30, 30);

    glViewport(0, height * 2 / 6, width / 4, height / 6);

    // --- Black ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.01f;
    materialDiffuse[1] = 0.01f;
    materialDiffuse[2] = 0.01f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.5f;
    materialSpecular[1] = 0.5f;
    materialSpecular[2] = 0.5f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.25f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[12], 5.0f, 30, 30);

    glViewport(width / 4, height * 2 / 6, width / 4, height / 6);

    // --- Cyan ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.1f;
    materialAmbient[2] = 0.06f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.0f;
    materialDiffuse[1] = 0.50980392f;
    materialDiffuse[2] = 0.50980392f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.50196078f;
    materialSpecular[1] = 0.50196078f;
    materialSpecular[2] = 0.50196078f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.25f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[13], 5.0f, 30, 30);

    glViewport(width * 2 / 4, height * 2 / 6, width / 4, height / 6);

    // --- Green ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.1f;
    materialDiffuse[1] = 0.35f;
    materialDiffuse[2] = 0.1f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.45f;
    materialSpecular[1] = 0.55f;
    materialSpecular[2] = 0.45f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.25f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[14], 5.0f, 30, 30);

    glViewport(width * 3 / 4, height * 2 / 6, width / 4, height / 6);

    // --- Red ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.5f;
    materialDiffuse[1] = 0.0f;
    materialDiffuse[2] = 0.0f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.6f;
    materialSpecular[2] = 0.6f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.25f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[15], 5.0f, 30, 30);

    glViewport(0, height / 6, width / 4, height / 6);

    // --- White ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.55f;
    materialDiffuse[1] = 0.55f;
    materialDiffuse[2] = 0.55f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.7f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.25f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[16], 5.0f, 30, 30);

    glViewport(width / 4, height / 6, width / 4, height / 6);

    // --- Yellow Plastic ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.5f;
    materialDiffuse[1] = 0.5f;
    materialDiffuse[2] = 0.0f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.6f;
    materialSpecular[1] = 0.6f;
    materialSpecular[2] = 0.5f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.25f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[17], 5.0f, 30, 30);

    glViewport(width * 2 / 4, height / 6, width / 4, height / 6);

    // --- Black ---
    materialAmbient[0] = 0.02f;
    materialAmbient[1] = 0.02f;
    materialAmbient[2] = 0.02f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.01f;
    materialDiffuse[1] = 0.01f;
    materialDiffuse[2] = 0.01f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.4f;
    materialSpecular[1] = 0.4f;
    materialSpecular[2] = 0.4f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.078125f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[18], 5.0f, 30, 30);

    glViewport(width * 3 / 4, height / 6, width / 4, height / 6);

    // --- Cyan ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.05f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.4f;
    materialDiffuse[1] = 0.5f;
    materialDiffuse[2] = 0.5f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.04f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.7f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.078125f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[19], 5.0f, 30, 30);

    glViewport(0, 0, width / 4, height / 6);

    // --- Green ---
    materialAmbient[0] = 0.0f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.4f;
    materialDiffuse[1] = 0.5f;
    materialDiffuse[2] = 0.4f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.04f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.078125f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[20], 5.0f, 30, 30);

    glViewport(width / 4, 0, width / 4, height / 6);

    // --- Red ---
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.0f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.5f;
    materialDiffuse[1] = 0.4f;
    materialDiffuse[2] = 0.4f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.04f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.078125f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[21], 5.0f, 30, 30);

    glViewport(width * 2 / 4, 0, width / 4, height / 6);

    // --- White ---
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.05f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.5f;
    materialDiffuse[1] = 0.5f;
    materialDiffuse[2] = 0.5f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.7f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.078125f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[22], 5.0f, 30, 30);

    glViewport(width * 3 / 4, 0, width / 4, height / 6);

    // --- Yellow Rubber ---
    materialAmbient[0] = 0.05f;
    materialAmbient[1] = 0.05f;
    materialAmbient[2] = 0.0f;
    materialAmbient[3] = 1.0f;

    materialDiffuse[0] = 0.5f;
    materialDiffuse[1] = 0.5f;
    materialDiffuse[2] = 0.4f;
    materialDiffuse[3] = 1.0f;

    materialSpecular[0] = 0.7f;
    materialSpecular[1] = 0.7f;
    materialSpecular[2] = 0.04f;
    materialSpecular[3] = 1.0f;

    materialShininess   = 0.078125f * 128.0f;

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(xCentre, yCentre, 0.0f);
    gluSphere(quadric[23], 5.0f, 30, 30);

}

void uninitialize(void)
{
    //variable declarations
    GLXContext currentGLXContext;
    
    //code
    for(int i = 0; i < 24; i++)
    {
        gluDeleteQuadric(quadric[i]);
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




