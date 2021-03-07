//headers
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include <math.h>
#include "RESOURCES.h"

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")


//macros
#define WIN_WIDTH  800                                  //window width
#define WIN_HEIGHT 600                                  //window height
#define PI         3.141592f                            //constant


//global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//global variable declaration
FILE *gpFile = NULL;                                     //log file

bool gbFullscreen = false;                               //toggling fullscreen
HWND ghwnd = NULL;                                       //global hwnd
DWORD dwStyle;                                           //window style
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };    //window placement before fullscreen

bool gbActiveWindow = false;                             //render only if window is active
HDC ghdc = NULL;                                         //current device context
HGLRC ghrc = NULL;                                       //rendering context


//WinMain() - entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function prototypes
    void Initialize(void);
    void Display(void);

    //variable declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyApp");
    int cxScreen, cyScreen;
    bool bDone = false;

    //code
    //create/open 'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Cannot open 'log.txt' file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "'log.txt' file created successfully.\nProgram started successfully.\n\n");
    }

    //initialization of WNDCLASSEX
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;
    wndclass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    //register above class
    RegisterClassEx(&wndclass);

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    //create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        szAppName,
        TEXT("OpenGL : Static INDIA"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        (cxScreen - WIN_WIDTH) / 2,
        (cyScreen - WIN_HEIGHT) / 2,
        WIN_WIDTH,
        WIN_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ghwnd = hwnd;
    
    Initialize();

    //show window
    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //game loop
    while(bDone == false)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                    bDone = true;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(gbActiveWindow == true)
            {
                //here you should call update function for OpenGL rendering

                //here you should call display function for OpenGL rendering
                Display();
            }
        }
        
    }

    return ((int)msg.wParam);
}


//WndProc() - callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //function prototypes
    void ToggleFullscreen(void);
    void Resize(int, int);
    void UnInitialize(void);

    //code
    switch(iMsg)
    {              
        case WM_SETFOCUS:
            gbActiveWindow = true;
            break;
        
        case WM_KILLFOCUS:
            gbActiveWindow = false;
            break;

        case WM_ERASEBKGND:
            return (0);

        case WM_SIZE:
            Resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case 0x46:
                    //fall through
                case 0x66:
                    ToggleFullscreen();
                    break;
                
                default:
                    break;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            UnInitialize();
            PostQuitMessage(0);
            break;
        
        default:
            break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}


//ToggleFullscreen() - toggle fullscreen
void ToggleFullscreen(void)
{
    //variable declaration
    MONITORINFO mi = { sizeof(MONITORINFO) };

    //code
    if(gbFullscreen == false)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }

        ShowCursor(false);
        gbFullscreen = true;
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
        ShowCursor(true);
        gbFullscreen = false;
    }
    
}


//Initialize() - initializes rendering context
void Initialize(void)
{
    //function prototypes
    void Resize(int, int);

    //variable declaration
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    //code
    ghdc = GetDC(ghwnd);

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    //initialization of PIXELFORMATDESCRIPTOR
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion    = 1;
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType  = PFD_TYPE_RGBA;
    pfd.cColorBits  = 32;
    pfd.cRedBits    = 8;
    pfd.cBlueBits   = 8;
    pfd.cGreenBits  = 8;
    pfd.cAlphaBits  = 8;
    pfd.cDepthBits  = 32;

    //choose required pixel format from device context
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        fprintf(gpFile, "ChoosePixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set that pixel format as current
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed.\n");
        DestroyWindow(ghwnd);
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed.\n");
        DestroyWindow(ghwnd);
    }

    //warm-up call to Resize()
    Resize(WIN_WIDTH, WIN_HEIGHT);

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
 
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}


//Resize() 
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


//Display() - renders scene
void Display(void)
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
        for(float angle = 0.0f; angle <= PI; angle += (PI / 17.0f))
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
        for(float angle = 0.0f; angle <= PI; angle += (PI / 17.0f))
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

    glBegin(GL_QUADS);

    glEnd();

    SwapBuffers(ghdc);
}


//UnInitialize()
void UnInitialize(void)
{
    //code
    if(gbFullscreen == true)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    
        ShowCursor(true);
    }

    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    if(gpFile)
    {
        fprintf(gpFile, "\nProgram completed successfully.\n'log.txt' file closed successfully.\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
