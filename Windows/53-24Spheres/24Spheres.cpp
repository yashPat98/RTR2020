// ------------------------
// Name :        Yash Patel
// Assignment :  24 Spheres Assignment 
// Date :        13-12-2020
// ------------------------

// --- Headers ---
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include "24Spheres.h"

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

bool gbLight            = false;
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
        TEXT("OpenGL : 24 Spheres"),
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
                case 'l':
                    //fall through
                case 'L':
                    if(gbLight == false)
                    {
                        //disable lighting
                        glEnable(GL_LIGHTING);
                        gbLight = true;
                    }
                    else
                    {
                        //enable lighting
                        glDisable(GL_LIGHTING);
                        gbLight = false;
                    }
                    break;

                case 'x':
                    //fall through
                case 'X':
                    key_pressed = 1;
                    angle_for_x_rotation = 0.0f;
                    break;
                
                case 'y':
                    //fall through
                case 'Y':
                    key_pressed = 2;
                    angle_for_y_rotation = 0.0f;
                    break;

                case 'z':
                    //fall through
                case 'Z':
                    key_pressed = 3;
                    angle_for_z_rotation = 0.0f;
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
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    //smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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

// --- Display() - renders scene ---
void Display(void)
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
    
    SwapBuffers(ghdc);
}

void DrawSpheres(void)
{
    //variable declaration
    GLfloat materialAmbient[4];
    GLfloat materialDiffuse[4];
    GLfloat materialSpecular[4];
    GLfloat materialShininess;

    //code
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

    glTranslatef(1.5f, 14.0f, 0.0f);
    gluSphere(quadric[0], 1.0f, 30, 30);

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

    glTranslatef(9.66f, 14.0f, 0.0f);
    gluSphere(quadric[1], 1.0f, 30, 30);

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

    glTranslatef(18.33f, 14.0f, 0.0f);
    gluSphere(quadric[2], 1.0f, 30, 30);

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

    glTranslatef(26.0f, 14.0f, 0.0f);
    gluSphere(quadric[3], 1.0f, 30, 30);

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

    glTranslatef(1.5f, 11.5f, 0.0f);
    gluSphere(quadric[4], 1.0f, 30, 30);

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

    glTranslatef(9.66f, 11.5f, 0.0f);
    gluSphere(quadric[5], 1.0f, 30, 30);

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

    glTranslatef(18.33f, 11.5f, 0.0f);
    gluSphere(quadric[6], 1.0f, 30, 30);

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

    glTranslatef(26.0f, 11.5f, 0.0f);
    gluSphere(quadric[7], 1.0f, 30, 30);

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

    glTranslatef(1.5f, 9.0f, 0.0f);
    gluSphere(quadric[8], 1.0f, 30, 30);

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

    glTranslatef(9.66f, 9.0f, 0.0f);
    gluSphere(quadric[9], 1.0f, 30, 30);

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

    glTranslatef(18.33f, 9.0f, 0.0f);
    gluSphere(quadric[10], 1.0f, 30, 30);

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

    glTranslatef(26.0f, 9.0f, 0.0f);
    gluSphere(quadric[11], 1.0f, 30, 30);

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

    glTranslatef(1.5f, 6.5f, 0.0f);
    gluSphere(quadric[12], 1.0f, 30, 30);

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

    glTranslatef(9.66f, 6.5f, 0.0f);
    gluSphere(quadric[13], 1.0f, 30, 30);

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

    glTranslatef(18.33f, 6.5f, 0.0f);
    gluSphere(quadric[14], 1.0f, 30, 30);

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

    glTranslatef(26.0f, 6.5f, 0.0f);
    gluSphere(quadric[15], 1.0f, 30, 30);

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

    glTranslatef(1.5f, 4.0f, 0.0f);
    gluSphere(quadric[16], 1.0f, 30, 30);

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

    glTranslatef(9.66f, 4.0f, 0.0f);
    gluSphere(quadric[17], 1.0f, 30, 30);

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

    glTranslatef(18.33f, 4.0f, 0.0f);
    gluSphere(quadric[18], 1.0f, 30, 30);

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

    glTranslatef(26.0f, 4.0f, 0.0f);
    gluSphere(quadric[19], 1.0f, 30, 30);

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

    glTranslatef(1.5f, 1.5f, 0.0f);
    gluSphere(quadric[20], 1.0f, 30, 30);

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

    glTranslatef(9.66f, 1.5f, 0.0f);
    gluSphere(quadric[21], 1.0f, 30, 30);

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

    glTranslatef(18.33f, 1.5f, 0.0f);
    gluSphere(quadric[22], 1.0f, 30, 30);

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

    glTranslatef(26.0f, 1.5f, 0.0f);
    gluSphere(quadric[23], 1.0f, 30, 30);
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

    //delete quadric objects
    for(int i = 0; i < 24; i++)
    {
        gluDeleteQuadric(quadric[i]);
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
