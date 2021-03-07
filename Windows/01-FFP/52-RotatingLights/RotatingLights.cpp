// ------------------------
// Name :        Yash Patel
// Assignment :  Rotating Lights assignment 
// Date :        12-12-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "RotatingLights.h"

#define _USE_MATH_DEFINES
#include <math.h>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// --- Macros ---
#define WIN_WIDTH  800                                   //window width
#define WIN_HEIGHT 600                                   //window height

// --- Global Function Declaration ---
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);    //callback function

// --- Global Variables ---
DWORD dwStyle;                                           //window style
FILE *gpFile            = NULL;                          //log file
HWND ghwnd              = NULL;                          //global hwnd
HDC ghdc                = NULL;                          //current device context
HGLRC ghrc              = NULL;                          //rendering context
WINDOWPLACEMENT wpPrev  = { sizeof(WINDOWPLACEMENT) };   //window placement before fullscreen

bool gbFullscreen       = false;                         //toggling fullscreen
bool gbActiveWindow     = false;                         //render only if window is active

GLfloat lightAmbientZero[]   = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseZero[]   = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecularZero[]  = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPositionZero[]  = {0.0f, 0.0f, 0.0f, 1.0f}; 

GLfloat lightAmbientOne[]    = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseOne[]    = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightSpecularOne[]   = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightPositionOne[]   = {0.0f, 0.0f, 0.0f, 1.0f}; 

GLfloat lightAmbientTwo[]    = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseTwo[]    = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightSpecularTwo[]   = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightPositionTwo[]   = {0.0f, 0.0f, 0.0f, 1.0f}; 

GLfloat materialAmbient[]    = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialDiffuse[]    = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[]   = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess    = 128.0f;

GLfloat lightAngle0          = 0.0f;
GLfloat lightAngle1          = 0.0f;
GLfloat lightAngle2          = 0.0f;

GLUquadric *quadric          = NULL;
bool gbLight                 = false;

// --- WinMain() - entry point function ---
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
        TEXT("OpenGL : Rotating Lights"),
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
                Display();
            }
        }
        
    }

    return ((int)msg.wParam);
}

// --- WndProc() - callback function ---
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

        case WM_CHAR:
            switch(wParam)
            {
                case 'L':
                case 'l':
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
                
                default:
                    break;
            }
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

// --- ToggleFullscreen() - toggle fullscreen ---
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

// --- Initialize() - initializes rendering context ---
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

    // --- Setup Render Scene ---

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //set up light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbientZero);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuseZero);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecularZero);
    glEnable(GL_LIGHT0);

    //set up light 1
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOne);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuseOne);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecularOne);
    glEnable(GL_LIGHT1);

    //set up light 2
    glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbientTwo);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuseTwo);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecularTwo);
    glEnable(GL_LIGHT2);

    //set up material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

    //warm-up call to Resize()
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

// --- Resize() --- 
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

// --- Display() - renders scene ---
void Display(void)
{
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
        //camera
        gluLookAt(0.0f, 0.0f, 2.0f,
                  0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f);  

        //light 0
        glPushMatrix();
            glRotatef(lightAngle0, 1.0f, 0.0f, 0.0f);
            
            lightPositionZero[1] = lightAngle0;
            glLightfv(GL_LIGHT0, GL_POSITION, lightPositionZero);
        glPopMatrix();

        //light 1
        glPushMatrix();
            glRotatef(lightAngle1, 0.0f, 1.0f, 0.0f);

            lightPositionOne[0] = lightAngle1;
            glLightfv(GL_LIGHT1, GL_POSITION, lightPositionOne);
        glPopMatrix();

        //light 2
        glPushMatrix();
            glRotatef(lightAngle2, 0.0f, 0.0f, 1.0f);

            lightPositionTwo[0] = lightAngle2;
            glLightfv(GL_LIGHT2, GL_POSITION, lightPositionTwo);
        glPopMatrix();

        glTranslatef(0.0f, 0.0f, -1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        quadric = gluNewQuadric();
        gluSphere(quadric, 0.5f, 30, 30);
    glPopMatrix();

    //update
    //light 0
    lightAngle0 = lightAngle0 + 0.02f;
    if(lightAngle0 > 360.0f)
        lightAngle0 = 0.0f;

    //light 1
    lightAngle1 = lightAngle1 + 0.02f;
    if(lightAngle1 > 360.0f)
        lightAngle1 = 0.0f;
    
    //light 2
    lightAngle2 = lightAngle2 + 0.02f;
    if(lightAngle2 > 360.0f)
        lightAngle2 = 0.0f;

    SwapBuffers(ghdc);
}

// --- UnInitialize() ---
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

    //delete quadric object
    gluDeleteQuadric(quadric);

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
