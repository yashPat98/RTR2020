//headers
#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include <math.h>
#include "RESOURCES.h"

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")


//macros
#define WIN_WIDTH     800                               //window width
#define WIN_HEIGHT    600                               //window height

#define MAX_PARTICLES 500                               //no of particles
#define PI            3.141592f                         //constant


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


//WinMain() - entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function prototypes
    void Initialize(void);
    void Display(void);
    void ToggleFullscreen(void);

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
        TEXT("OpenGL : Dynamic INDIA"),
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

    ToggleFullscreen();
    PlaySound(MAKEINTRESOURCE(MYSOUND), NULL, SND_ASYNC | SND_RESOURCE);

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
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);

    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);

    //initialize particle
    for(int loop = 0; loop < MAX_PARTICLES; loop++)
    {
        particles[loop].active = true;
        particles[loop].life = 1.0f;
        particles[loop].fade = (float)(rand() % 100) / 5000.0f + 0.003f;

        particles[loop].rotation = (((float)rand() / (float)RAND_MAX) * 25.0f) - 10.0f;
        particles[loop].size = (((float)rand() / (float)RAND_MAX) * 15.0f);

        particles[loop].x = 0.0f;
        particles[loop].y = 0.0f;

        particles[loop].xi = (float)(rand() % 100) / 2000.0f + 0.05f;
    }
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
    //function declaration
    void DrawIndia(void);
    void DrawJet(int jetNum);

    //variable declaration
    static bool bRender = false;
    static int frameCount = 0;

    static GLfloat JetTrans = -83.0f;
    static GLfloat xParabola = -83.0f;
    static GLfloat xRot = -50.0f;
    GLfloat yParabola = 0.0f;
    GLfloat step = 0.083f;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        if(JetTrans >= 42.0f)
            gbHoverPlane = true;

        if(JetTrans <= 150.0f)
            JetTrans = JetTrans + 0.1f;

        if(gbRotation)
            xRot += step;
        else if(yParabola > 4.1f)
        {
            gbRotation = true;
            step = -step;
        }
        
        xParabola += 0.1f;
    }

    SwapBuffers(ghdc);
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
    float speed = 0.03f;

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
            nTrans -= (speed + 0.03f);
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
    //function declaration
    void Smoke(int jetNum);

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


    glTranslatef(-12.0f, 0.0f, 0.0f);
    Smoke(jetNum);
}

void Smoke(int jetNum)
{
    //variable declaration
    static int slowdown = 0;
    GLfloat r;
    GLfloat g;
    GLfloat b;

    if(jetNum == 1)
    {
        r = 1.0f;
        g = 0.55859f;
        b = 0.10937f;
    }
    else if(jetNum == 2)
    {
        r = 1.0f;
        g = 1.0f;
        b = 1.0f;
    }
    else if(jetNum == 3)
    {
        r = 0.3125f;
        g = 0.61718f;
        b = 0.18359f;
    }else
    {
        return;
    }

    //code
    for(int loop = 0; loop < MAX_PARTICLES; loop++)
    {
        glPointSize(particles[loop].size);

        glPushMatrix();
            glRotatef(particles[loop].rotation, 0.0f, 0.0f, 1.0f);    

            glColor4f(r, g, b, particles[loop].life);
            glBegin(GL_POINTS);
                glVertex3f(particles[loop].x, particles[loop].y, 0.0f);
            glEnd();
        glPopMatrix();

        particles[loop].life -= particles[loop].fade;
        particles[loop].size -= particles[loop].fade;

        particles[loop].x -= particles[loop].xi; 

        if(particles[loop].life < 0.0f)
        {
            particles[loop].life = 1.0f;
            particles[loop].fade = (float)(rand() % 100) / 5000.0f + 0.003f;

            particles[loop].rotation = (((float)rand() / (float)RAND_MAX) * 25.0f) - 10.0f;
            particles[loop].size = 15.0f;

            particles[loop].x = 0.0f;
            particles[loop].y = 0.0f;

            particles[loop].xi = (float)(rand() % 100) / 1000.0f + 0.05f;
        }
    }
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
