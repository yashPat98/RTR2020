//headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

//namespaces
using namespace std;

//global variable declaration
bool gbFullscreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;

int giWindowWidth = 800;
int giWindowHeight = 600;
char keys[26];

//entry-point function
int main(void)
{
    //function prototypes
    void CreateWindow(void);
    void ToggleFullscreen(void);
    void uninitialize(void);
    
    //variable declarations
    int winWidth = giWindowWidth;
    int winHeight = giWindowHeight;
    
    static XFontStruct *pxFontStruct = NULL; 
    static GC gc;
    XGCValues gcValues; 
    XColor greenColor;
    char str[] = "Hello XWindow From Yash Patel";
    int stringLength;
    int stringWidth;
    int fontHeight;
    
    //code
    CreateWindow();
    
    //Message Loop
    XEvent event;
    KeySym keysym;
    
    while(1)
    {
        XNextEvent(gpDisplay, &event);
        switch(event.type)
        {
            case MapNotify:
                pxFontStruct = XLoadQueryFont(gpDisplay, "fixed");
                break;
            
            case KeyPress:
                keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                switch(keysym)
                {
                    case XK_Escape:
                        XUnloadFont(gpDisplay, pxFontStruct->fid);
                        XFreeGC(gpDisplay, gc);
                        uninitialize();
                        exit(0);
                    
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
                    case 'a':
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
                break;
            
            case Expose:
                gc = XCreateGC(gpDisplay, gWindow, 0, &gcValues);
                XSetFont(gpDisplay, gc, pxFontStruct->fid);
                XAllocNamedColor(gpDisplay, gColormap, "green", &greenColor, &greenColor);
                XSetForeground(gpDisplay, gc, greenColor.pixel);
                
                stringLength = strlen(str);
                stringWidth = XTextWidth(pxFontStruct, str, stringLength);
                fontHeight = pxFontStruct->ascent + pxFontStruct->descent;
                XDrawString(gpDisplay, 
                            gWindow, 
                            gc, 
                            (winWidth / 2 - stringWidth / 2), 
                            (winHeight / 2 - fontHeight / 2), 
                            str, 
                            stringLength);
                break;
            
            case DestroyNotify:
                break;
            
            case 33:
                XUnloadFont(gpDisplay, pxFontStruct->fid);
                XFreeGC(gpDisplay, gc);
                uninitialize();
                exit(0);
                break;
            
            default:
                break;
        }
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
    int defaultDepth;
    int styleMask;
    
    //code
    gpDisplay = XOpenDisplay(NULL); 
    if(gpDisplay == NULL)
    {
        printf("ERROR : Unable To Open X Display.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    defaultScreen = XDefaultScreen(gpDisplay);
    defaultDepth = XDefaultDepth(gpDisplay, defaultScreen);
    
    gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR : Unable To Allocate Memory For Visual Info.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    Status status = XMatchVisualInfo(gpDisplay, defaultScreen, defaultDepth, TrueColor, gpXVisualInfo);
    if(status == 0)
    {
        printf("Error : Unable To Get A Visual.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    winAttribs.border_pixel = 0;
    winAttribs.background_pixmap = 0;
    winAttribs.background_pixel = XBlackPixel(gpDisplay, defaultScreen);
    winAttribs.colormap = XCreateColormap(gpDisplay,
                          RootWindow(gpDisplay, gpXVisualInfo->screen),
                          gpXVisualInfo->visual,
                          AllocNone);
    winAttribs.event_mask = KeyPressMask | ButtonPressMask | ExposureMask | 
                            StructureNotifyMask | PointerMotionMask | VisibilityChangeMask;
    
    gColormap = winAttribs.colormap;
    styleMask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
    
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
        printf("ERROR : Failed To Create Main Window.\nExitting Now...\n");
        uninitialize();
        exit(1);
    }
    
    XStoreName(gpDisplay, gWindow, "Hello XWindow");
    
    //Atom - unique immutable string representing number 
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
    
    xev.type = ClientMessage;
    xev.xclient.window = gWindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = gbFullscreen ? 0 : 1;
    
    fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[1] = fullscreen;
    
    XSendEvent(gpDisplay, 
               RootWindow(gpDisplay, gpXVisualInfo->screen),
               False,
               StructureNotifyMask,
               &xev);
}

void uninitialize(void)
{
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
