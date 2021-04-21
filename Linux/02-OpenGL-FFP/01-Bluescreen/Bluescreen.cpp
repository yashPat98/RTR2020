//All Xlib APIs are blocking or synchronous APIs.
//All Xlib APIs are passed in the queue and are dispatched to 
//XServer one by one but XServer can respond asynchronously
//the responses are then again cached by Xlib and passed to 
//XClient synchronously.

//Almost all Xlib APIs require pointer to Display structure.
//Display is data structure representing graphics device driver
//Display is data structure representing XServer
//Display is data structure which is interface between XServer and XClient

//API types
//Xfunction -> API
//function -> Macro

//headers
#include <iostream>                     //namespace (pool of header files)
#include <stdio.h>                      //standard c header (printf)
#include <stdlib.h>                     //standard c header (exit)
#include <memory.h>                     //standard c header (memset)

#include <X11/Xlib.h>                   //standard xwindow header
#include <X11/Xutil.h>                  //xwindow utility header (XVisualInfo)
#include <X11/XKBlib.h>                 //xwindow keyboard header (XkbKeycodeToKeysym)
#include <X11/keysym.h>                 //xwindow key symbol header (KeySym)

#include <GL/gl.h>                      //standard OpenGL header
#include <GL/glx.h>                     //OpenGL and xwindow bridging header

#define INIT_X        0                 //initial x-coordinate of top left corner of window
#define INIT_Y        0                 //initial y-coordinate of top left corner of window
#define BORDER_WIDTH  0                 //border width of the window

//namespaces
using namespace std;                    //use std namespace from iostream

//global variable declarations
Display      *gpDisplay      = NULL;    //interface between XServer and XClient
XVisualInfo  *gpXVisualInfo  = NULL;    //structure containing info of visual type
Colormap      gColormap;                //structure contating pixel color mapping of window
Window        gWindow;                  //structure representing window
GLXContext    gGLXContext;              //structure representing OpenGL rendering context

int giWindowWidth   = 800;              //initial width of the window
int giWindowHeight  = 600;              //initial height of the window

bool gbFullscreen = false;              //toggle window to fullscreen 

//entry-point function
int main(void)
{
    //function declarations
    void CreateWindow(void);            //create and map window to display
    void ToggleFullscreen(void);        //toggle window to fullscreen 
    void Initialize(void);              //initialize OpenGL context and setup render scene
    void Resize(int, int);              //resize viewport and projection matrix 
    void Render(void);                  //render the scene
    void uninitialize(void);            //cleanup
    
    //variable declarations
    int winWidth = giWindowWidth;       //new window width
    int winHeight = giWindowHeight;     //new window height
    bool bDone = false;                 //game loop variable
    
    //code
    //create and map the window to display
    CreateWindow();                     
    
    //initialize the OpenGL context and 
    //setup the render scene
    Initialize();
    
    //Game Loop
    XEvent event;                       //union containing info of events recieved by window
    KeySym keysym;                      //encoding of a symbol on the cap of a key
    
    while(bDone == false)
    {
        //XPending() function returns the number of events (unsigned int) that have been 
        //recieved from X server but have not been removed from event queue
        //if there are no events in the queue it will return 0
        while(XPending(gpDisplay))
        {
            //get the next event in event union from XServer
            XNextEvent(gpDisplay, &event);
        
            //handle the event
            switch(event.type)
            {
                case MapNotify:             //event : window is mapped to display
                    break;
            
                case KeyPress:              //event : a key is pressed
                    //convert the keyboard's raw keycode to key symobol
                    // 1st param : specifies the connection to the XServer.
                    // 2nd param : keycode from the event
                    // 3rd param : keycode group number (0 : default)
                    // 4th param : state of shift key 
                    keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                    switch(keysym)
                    {
                        case XK_Escape:     //escape key is pressed
                            //exit from game loop
                            bDone = true;
                            break;
                    
                        case XK_F:          //F key is pressed
                        case XK_f:          //f key is pressed
                            //toggle window to fullscreen and vise versa
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
                        
                        default:
                            break;
                    }
                    break;
            
                case ButtonPress:            //event : mouse button is pressed
                    switch(event.xbutton.button)
                    {
                        case 1:             //left mouse button is pressed
                            break; 
                     
                        case 2:             //middle mouse button is pressed
                            break;
                     
                        case 3:             //right mouse button is pressed
                            break;
                    
                        default:
                            break;
                    }
                    break;
                
                case MotionNotify:          //event : window is moved
                    break;
            
                case ConfigureNotify:       //event : window is resized
                    //get the new window width and height
                    winWidth = event.xconfigure.width;
                    winHeight = event.xconfigure.height;
                    Resize(winWidth, winHeight);
                    break;
                
                case Expose:                //event : window needs to repaint 
                    break;
                
                case DestroyNotify:         //event : window is destroyed
                    break;
            
                case 33:                    //event : close button is clicked (WM_DELETE_WINDOW) 
                    //exit from game loop
                    bDone = true;
                    break;
            
                default:
                    break;
            }
        }
        
        //render the scene if there
        //are no events to process
        Render();
    }
    
    //cleanup 
    uninitialize();
    
    //successful return
    return (0);
}

void CreateWindow(void)
{
    //function declarations
    void uninitialize(void);            //cleanup
    
    //variable declarations
    XSetWindowAttributes winAttribs;    //window attributes or properties
    
    int defaultScreen;                  //default screen number 
    int styleMask;                      //window style mask
    
    //conventionally static
    static int frameBufferAttributes[] = { GLX_RGBA,              //pixel type
                                           GLX_RED_SIZE,   1,     //red color bits
                                           GLX_GREEN_SIZE, 1,     //green color bits
                                           GLX_BLUE_SIZE,  1,     //blue color bits
                                           GLX_ALPHA_SIZE, 1,     //alpha bits
                                           None };                //None or 0 indicates other attributes are unknown
                                           
    //code
    //open a connection with XServer 
    //NULL : don't configure XServer, connect to default 
    //       behaviour of XServer
    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        printf("Error : Failed To Open X Display.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    //get default screen number referenced by gpDisplay
    defaultScreen = XDefaultScreen(gpDisplay);
    
    //allocate memory for gpXVisualInfo
    gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));
    if(gpXVisualInfo == NULL)
    {
        printf("Error : Failed To Allocate Memory For XVisualInfo.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    //choose and return a visual which matches best with attributes in integer array
    gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
    if(gpXVisualInfo == NULL)
    {
        printf("Error : Failed To Get A Visual.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    //set attributes of window
    winAttribs.border_pixel       = 0;                                                  //default border color
    winAttribs.background_pixmap  = (Pixmap)0;                                          //no background image
    winAttribs.background_pixel   = XBlackPixel(gpDisplay,                              //background color
                                        defaultScreen);
    //for each possible value that a pixel can take in a window, 
    //there is a color cell in the colormap.
    //create a specific colormap for window
    // gpDisplay             : specifies the connection to the XServer.
    // RootWindow()          : prototype of window having required colormap 
    //                         get the RootWindow of TrueColor type (gpXVisualInfo->screen)
    // gpXVisualInfo->visual : a visual type supported on the screen.
    // AllocNone             : do not allocate memory for colormap 
    winAttribs.colormap           = XCreateColormap(gpDisplay,                          
                                        RootWindow(gpDisplay, gpXVisualInfo->screen),   
                                        gpXVisualInfo->visual,
                                        AllocNone);
    //events to be sent by XServer (XProtocol) to the window
    // ExposureMask          : window repaint events
    // VisibilityChangeMask  : window visibility events
    // ButtonPressMask       : mouse button events
    // KeyPressMask          : keyboard keys events
    // PointerMotionMask     : window move events
    // StructureNotifyMask   : window resize events
    winAttribs.event_mask         = ExposureMask | VisibilityChangeMask | ButtonPressMask |
                                    KeyPressMask | PointerMotionMask | StructureNotifyMask;
    
    //window style mask (CW - Create Window)
    // CWBackPixel    : window must have background color 
    // CWBorderPixel  : window must have border 
    // CWColormap     : window must have colormap 
    // CWEventMask    : window must handle events
    styleMask = CWBackPixel |CWBorderPixel | CWColormap | CWEventMask;
                                    
    //store colormap in global variable
    gColormap = winAttribs.colormap;
    
    //create the window
    gWindow = XCreateWindow(gpDisplay,                                 //specifies the connection to XServer
                    RootWindow(gpDisplay, gpXVisualInfo->screen),      //parent window with TrueColor visual
                    INIT_X,                                            //x-coordinate of top left corner of window
                    INIT_Y,                                            //y-coordinate of top left corner of window
                    giWindowWidth,                                     //initial window width
                    giWindowHeight,                                    //initial window height
                    BORDER_WIDTH,                                      //border width of window (0 : default)
                    gpXVisualInfo->depth,                              //color depth of the window
                    InputOutput,                                       //handle input and output events
                    gpXVisualInfo->visual,                             //structure that contains info of color mapping
                    styleMask,                                         //window styles
                    &winAttribs);                                      //window attributes 
                
    if(!gWindow)
    {
        printf("Error : Failed To Create Main Window.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    //caption of the window
    XStoreName(gpDisplay, gWindow, "OpenGL Bluescreen");
    
    //exiting window from close button is window manager feature
    //Atom : unique immutable string representing a number (number for network compliency)
    //XInternAtom : encode string into Atom
    // 3rd Param  : True : create Atom even if it is cached before
    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    
    //set window manager protocol for close button event
    // 3rd Param : array of Atoms
    // 4th Param : number of Atoms in array
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);
    
    //map the window to display or show window
    XMapWindow(gpDisplay, gWindow);
}

void ToggleFullscreen(void)
{
    //variable declarations
    Atom wm_state;                            //Atom representing current window state
    Atom fullscreen;                          //Atom representing fullscreen mode 
    XEvent xev = {0};                         //union containing info of event
    
    //code
    //get Atom of current state of window
    // 2nd Param     : window message
    // _NET_WM_STATE : list of Atoms representing current window state
    //                 _NET_WM_STATE_MINIMIZED,
    //                 _NET_WM_STATE_MAXIMIZED,
    //                 _NET_WM_STATE_FULLSCREEN,
    //                 etc
    // 3rd Param : don't create Atom if previously cached
    wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    
    //zero out memory
    memset(&xev, 0, sizeof(xev));
    
    //initialize the event
    xev.type                  = ClientMessage;          //type of the event
    xev.xclient.window        = gWindow;                //respective client window
    xev.xclient.message_type  = wm_state;               //type of the message (list of current state of window)
    xev.xclient.format        = 32;                     //format of the data of message (32 bit)
    
    //set the long data
    //data.l[0] = action 
    //data.l[1] = property to alter
    //data.l[2] = property to alter
    
    //action applied on list of Atoms :
    // _NET_WM_STATE_REMOVE  0
    // _NET_WM_STATE_ADD     1
    // _NET_WM_STATE_TOGGLE  2  
    
    xev.xclient.data.l[0]     = gbFullscreen ? 0 : 1;   //action to apply on property
    
    //create a fullscreen mode Atom
    fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[1]     = fullscreen;             //first property to alter
    
    //send the event to window
    // 1st param : specify the connection with XServer
    // 2nd param : specify the window the event is to sent
    // 3rd param : propogate the event to child window or not
    // 4th param : specify the event mask
    // 5th param : specify the event to be sent
    XSendEvent(gpDisplay, 
               RootWindow(gpDisplay, gpXVisualInfo->screen),
               False,
               StructureNotifyMask,
               &xev);
}

void Initialize(void)
{
    //function prototypes
    void Resize(int, int);              
    
    //code
    //create a GLX rendering context and get its handle
    // 1st param -> specifies connection with X server
    // 2nd param -> specifies a visual info which contains the visual that 
    //              defines frame buffer resources available to the rendering context
    // 3rd param -> specifies the context with which to share Display list
    //              NULL indicates no sharing
    // 4th param -> specific whether rendering to done with direct connection to
    //              graphics system (true) or through x server (Mesa 3D software renderer) (false)
    gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
    
    //set the GLXContext as current rendering context
    //replacing previous if there was one
    // 1st param -> specifies connection with X server
    // 2nd param -> specifies the GLX drawable (i.e window or pixmap) 
    //              it also attaches the context with the window
    // 3rd param -> specifies OpenGL rendering context to be set as current
    // Note : glXMakeCurrent() also creates initial viewport of drawable size
    //        i.e window size
    glXMakeCurrent(gpDisplay, gWindow, gGLXContext);
    
    //set color buffer clearing color
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    
    //warm-up call
    Resize(giWindowWidth, giWindowHeight);
}

void Resize(int width, int height)
{
    //code 
    //avoid divide by 0 error
    if(height == 0)
        height = 1;
    
    //set the viewport 
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void Render(void)
{
    //code
    //clear the color buffer 
    glClear(GL_COLOR_BUFFER_BIT);
    
    //flush the buffers causing all the
    //issued command to be executed by renderer 
    glFlush();
}

void uninitialize(void)
{
    //variable declaration
    GLXContext currentGLXContext;           
    
    //code
    //get the current rendering context
    currentGLXContext = glXGetCurrentContext();
    if(currentGLXContext == gGLXContext)
    {
        // 2nd param -> 0 indicates detach the GLXContext from window
        // 3rd param -> 0 indicates set default context as current
        glXMakeCurrent(gpDisplay, 0, 0);
    }
    
    //release the rendering context
    if(gGLXContext)
    {
        glXDestroyContext(gpDisplay, gGLXContext);
    }
    
    //destroy the window
    if(gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
    }
    
    //free the colormap 
    if(gColormap)
    {
        XFreeColormap(gpDisplay, gColormap);
    }
    
    //release gpXVisualInfo
    if(gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }
    
    //Close the connection with XServer
    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }
    
}
