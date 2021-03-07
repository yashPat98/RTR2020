// ------------------------
// Name :        Yash Patel
// Assignment :  FFP Final Project 
// Date :        29-12-2020
// ------------------------

// --- Headers ---
#include "Destiny.h"
#include "Atmosphere.h"
#include "Models.h"
#include "DataStructure.h"

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")

// --- Macros ---
#define WIN_WIDTH  800                                   //window width
#define WIN_HEIGHT 600                                   //window height

#define MAX_TREES  350
#define MAX_HUMANOIDS 6

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

GLuint groundList;                                       //ground display list
GLuint treeList;                                         //large christmas tree display list
GLuint fontList;                                         //font display list
GLuint shadowList;                                       //display list for shadow

GLuint ground_texture;                                   
GLuint day_texture;                                      
GLuint night_texture;                                    
GLuint feather_texture;
GLuint bird_texture;

GLfloat lightPosition[] = {70.0f, 70.0f, -100.0f, 1.0f};
GLfloat lightAmbient[]  = {0.35f, 0.35f, 0.35f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

GLfloat shadowMat[16];

Humanoid_Appearance jackLooks, humanoidLooks[MAX_HUMANOIDS];
Humanoid_Animation jackProp, humanoidProp;

float tree_position_X[MAX_TREES];
float tree_position_Z[MAX_TREES];

bool gbStart = false;
bool gbOpeningCredits = true;
bool gbScene1 = false;
bool gbScene2 = false;
bool gbScene3 = false;
bool gbScene4 = false;
bool gbScene5 = false;
bool gbEndCredits = false;
bool gbSpecialCredits = false;

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
        TEXT("OpenGL : Destiny"),
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

                case 0x53:
                    //fall through
                case 0x73:
                    if(gbStart == false)
                    {
                        gbStart = true;
                        PlaySound(MAKEINTRESOURCE(BG_MUSIC), NULL, SND_ASYNC | SND_RESOURCE);
                    }
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
    bool loadGLTexture(GLuint *texture, TCHAR ResourceID[]);
    void SetupFontRendering(void);
    void RenderGround(void);
    void myShadowMatrix(float *proj, float *ground, float *light);

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

    //variable declaration
    GLfloat point1[] = {0.0f, -0.4f, 0.0f};
    GLfloat point2[] = {10.0f, -0.4f, 0.0f};
    GLfloat point3[] = {5.0f, -0.4f, -5.0f};
    GLfloat planeEquation[4];

    GLfloat fogColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    
    struct Node *head = NULL;
    struct Node *temp = NULL;
    int i;

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    //depth
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //back face culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);

    //enable lighting and material properties
    glEnable(GL_LIGHTING);

    //set up light 1
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.08f);
    glEnable(GL_LIGHT1);

    //enable color tracking
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    //enable texture memory
    glEnable(GL_TEXTURE_2D);

    //load textures
    loadGLTexture(&ground_texture, MAKEINTRESOURCE(GROUND_BITMAP));
    loadGLTexture(&day_texture, MAKEINTRESOURCE(DAY_BITMAP));
    loadGLTexture(&night_texture, MAKEINTRESOURCE(NIGHT_BITMAP));
    loadGLTexture(&feather_texture, MAKEINTRESOURCE(FEATHER_BITMAP));
    loadGLTexture(&bird_texture, MAKEINTRESOURCE(BIRD_BITMAP));

    //set up fog properties
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.03f);
    glFogf(GL_FOG_START, 15.0f);
    glFogf(GL_FOG_END, 20.0f);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glEnable(GL_FOG);

    //construct shadow projection matrix
    GetPlaneEquation(planeEquation, point1, point2, point3);
    ShadowMatrix(shadowMat, planeEquation, lightPosition);

    //setup font rendering
    SetupFontRendering();

    //create linked list for position of trees
    head = CreateList(head, MAX_TREES);
    head = DeleteElementsInRange(head, -3.0f, 3.0f, -15.0, 15.0);

    //initialize humanoids
    InitHumanoid(&humanoidProp);
    for(i = 0; i < MAX_HUMANOIDS; i++)
    {
        humanoidLooks[i].bShadow = true;
        humanoidLooks[i].bChains = true;
        humanoidLooks[i].x = (float)((rand() % 400) - 200) * 0.01f;
        humanoidLooks[i].z = (float)((rand() % 200) - 100) * 0.1f;
        humanoidLooks[i].shirtColor[0] = (float)(rand() % 1000) * 0.001f;
        humanoidLooks[i].shirtColor[1] = (float)(rand() % 1000) * 0.001f;
        humanoidLooks[i].shirtColor[2] = (float)(rand() % 1000) * 0.001f;
        humanoidLooks[i].pantColor[0] = (float)(rand() % 500) * 0.001f;
        humanoidLooks[i].pantColor[1] = (float)(rand() % 600) * 0.001f;
        humanoidLooks[i].pantColor[2] = (float)(rand() % 400) * 0.001f;
    }

    //initialize main actor
    InitHumanoid(&jackProp);
    jackLooks.bShadow = true;
    jackLooks.bChains = true;
    jackLooks.x = 0.0f;
    jackLooks.z = 0.0f;
    jackLooks.shirtColor[0] = 0.6875f;
    jackLooks.shirtColor[1] = 0.6875f;
    jackLooks.shirtColor[2] = 0.6875f;
    jackLooks.pantColor[0] = 0.5f; 
    jackLooks.pantColor[1] = 0.45703f; 
    jackLooks.pantColor[2] = 0.35156f; 

    //generate and initialize display lists
    groundList = glGenLists(1);
    glNewList(groundList, GL_COMPILE);
        glBindTexture(GL_TEXTURE_2D, ground_texture);
        RenderGround();
    glEndList();

    treeList = glGenLists(1);
    glNewList(treeList, GL_COMPILE);
        //draw shadow first
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        i = 0;
        temp = head;
        while(temp != NULL)
        {
            glPushMatrix();
                glTranslatef((GLfloat)temp->x, (GLfloat)temp->y, (GLfloat)temp->z);
                glMultMatrixf(shadowMat);

                if((i % 2) == 0)
                    RenderChristmasTreeLarge(true);
                else
                    RenderChristmasTreeSmall(true);
            glPopMatrix();

            temp = temp->next;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);

        //draw trees
        i = 0;
        temp = head;
        while(temp != NULL)
        {
            glPushMatrix();
                glTranslatef((GLfloat)temp->x, (GLfloat)temp->y, (GLfloat)temp->z);

                if((i % 2) == 0)
                    RenderChristmasTreeLarge(false);
                else
                    RenderChristmasTreeSmall(false);
            glPopMatrix();

            temp = temp->next;
        }
    glEndList();

    shadowList = glGenLists(1);
    glNewList(shadowList, GL_COMPILE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glMultMatrixf(shadowMat);
        RenderHumanoid(&humanoidProp, humanoidLooks[0]);
       
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
    glEndList();
    
    //free memory
    head = DeleteList(head);
    temp = NULL;

    //warm-up call to Resize()
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

// --- loadGLTexture() - loads texture from resource ---
bool loadGLTexture(GLuint *texture, TCHAR ResourceID[])
{
    //variable declaration
    bool bResult = false;
    HBITMAP hBitmap = NULL;
    BITMAP bmp;

    //code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if(hBitmap)
    {
        bResult = true;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        //generate texture object
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        //setting texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //push the data to texture memory
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
    
        //free bitmap object
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    return (bResult);
}

void SetupFontRendering(void)
{
    //variable declaration
    HFONT hFont;
    GLYPHMETRICSFLOAT agmf[128];
    LOGFONT logfont;

    //code
    //initialization of logfont
    logfont.lfHeight          = -10;
    logfont.lfWidth           = 0;
    logfont.lfEscapement      = 0;
    logfont.lfOrientation     = 0;
    logfont.lfWeight          = FALSE;
    logfont.lfItalic          = FALSE;
    logfont.lfUnderline       = FALSE;
    logfont.lfStrikeOut       = FALSE;
    logfont.lfCharSet         = ANSI_CHARSET;
    logfont.lfOutPrecision    = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality         = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily  = DEFAULT_PITCH;
    strcpy(logfont.lfFaceName, "Verdana");

    //create the font and display list
    hFont = CreateFontIndirect(&logfont);
    SelectObject(ghdc, hFont);

    fontList = glGenLists(128);
    wglUseFontOutlines(ghdc, 0, 128, fontList, 0.0f, 0.0f, WGL_FONT_POLYGONS, agmf);

    DeleteObject(hFont);
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
    //function declaration
    void OpeningCredits(void);
    void Scene1(void);
    void Scene2(void);
    void Scene3(void);
    void Scene4(void);
    void Scene5(void);
    void EndCredits(void);
    void SpecialCredits(void);

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(gbStart == true)
    {
        if(gbOpeningCredits)
            OpeningCredits();

        else if(gbScene1)
            Scene1();

        else if(gbScene2)
            Scene2();
            
        else if(gbScene3)
            Scene3();

        else if(gbScene4)
            Scene4();

        else if(gbScene5)
            Scene5();

        else if(gbEndCredits)
            EndCredits();

        else if(gbSpecialCredits)
            SpecialCredits();
    }

    SwapBuffers(ghdc);
}

void OpeningCredits(void)
{
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 1.0f;
    static bool bFade = false;

    static GLfloat yCam = 0.0f;
    static GLfloat xAMC = 10.0f;
    static GLfloat xPr = 25.0f;

    static bool bAMC = false;

    char *szASTROMEDICOMP = "ASTROMEDICOMP";
    char *szPresents = "Presents";

    //code
    glPushMatrix();
        //camera    
        gluLookAt(0.0f, yCam, 14.0f, 
                  0.0f, -10.5f + yCam, 8.0f,
                  0.0f, 1.0f, 0.0f);

        //lights
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

        //scene
        glCallList(groundList);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 0.0f, 0.0f);

        glPushMatrix();
            glMultMatrixf(shadowMat);    
            
            if(bAMC)
            {
                glPushMatrix();
                    glTranslatef(xAMC, 0.0f, 6.5f);
                    glScalef(1.5f, 1.5f, 1.5f);
                    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                    glListBase(fontList);
                    glCallLists(strlen(szASTROMEDICOMP), GL_UNSIGNED_BYTE, szASTROMEDICOMP);
                glPopMatrix();
     
                glPushMatrix();
                    glTranslatef(xPr, 0.0f, 7.5f);
                    glScalef(1.5f, 1.5f, 1.5f);
                    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                    glListBase(fontList);
                    glCallLists(strlen(szPresents), GL_UNSIGNED_BYTE, szPresents);
                glPopMatrix();
            }
        glPopMatrix();
        
        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);

        glCallList(treeList);
    glPopMatrix();

    //fade 
    Fade(fadeAlpha);

    //update 
    if(yCam < 10.5f)
        yCam += 0.007f;    

    if(yCam >= 5.0f)
        bAMC = true;

    if(bAMC)
    {
        if(xAMC >= -24.0f)
            xAMC -= 0.02f;
        
        if(xAMC <= -18.0f)
            bFade = true;

        if(xAMC <= -20.0f)
        {
            gbOpeningCredits = false;
            gbScene1 = true;
        }

        if(xPr >= -24.0f)
            xPr -= 0.03f;
    }

    //fade 
    if(bFade == false)
    {
        if(fadeAlpha >= 0.0f)
            fadeAlpha -= 0.01f;    
    }
    else
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.01f;
    }
    
}

void Scene1(void)
{   
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 1.0f;
    static bool bFade = false;
    
    static GLfloat humanTrans = 0.01f;

    static GLfloat yEye = 2.0f;
    static GLfloat zEye = 18.0f;
    static GLfloat zInc = 0.01f;

    //code
    glBindTexture(GL_TEXTURE_2D, day_texture);
    RenderSky();

    glPushMatrix();
        //camera    
        gluLookAt(-1.5f, yEye, zEye, 
                  -1.5f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f);

        //lights
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

        //scene
        glCallList(groundList);   
        
        //draw shadow first
        for(int i = 0; i < MAX_HUMANOIDS; i++)
        {
            glPushMatrix();
                glTranslatef(humanoidLooks[i].x, 0.0f, humanoidLooks[i].z);
                if((i % 2) == 0)
                    glTranslatef(0.0f, 0.0f, -humanTrans);
                else
                    glTranslatef(0.0f, 0.0f, humanTrans);

                glCallList(shadowList);
            glPopMatrix();
        }

        //shadow for jack
        glPushMatrix();
            glTranslatef(-1.5f, 0.0f, humanTrans - 10.0f);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            jackLooks.bShadow = true;

            glMultMatrixf(shadowMat);
            RenderHumanoid(&jackProp, jackLooks);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
        glPopMatrix();

        glCallList(treeList);

        //draw humanoids
        for(int i = 0; i < MAX_HUMANOIDS; i++)
        {
            humanoidLooks[i].bShadow = false;
            glPushMatrix();
                glTranslatef(humanoidLooks[i].x, 0.0f, humanoidLooks[i].z);
                if((i % 2) == 0)
                    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

                glTranslatef(0.0f, 0.0f, humanTrans);
                RenderHumanoid(&humanoidProp, humanoidLooks[i]);
            glPopMatrix();
        }   

        //jack
        glPushMatrix();
            glTranslatef(-1.5f, 0.0f, humanTrans - 10.0f);

            jackLooks.bShadow = false;
            RenderHumanoid(&jackProp, jackLooks);
        glPopMatrix();
    glPopMatrix();

    //fade 
    Fade(fadeAlpha);

    //update 
    UpdateHumanoid(&humanoidProp);
    UpdateHumanoid(&jackProp);

    if(humanTrans <= 25.0f)
        humanTrans += 0.007f;
    
    //camera
    if(yEye >= 0.0f)
        yEye -= 0.001f;
    
    zEye -= zInc;
    if(zEye <= 2.0f)
        zInc = -zInc;

    if(zEye >= 14.0f && zInc < 0.0f)
        bFade = true;

    if(zEye >= 15.0f && zInc < 0.0f)
    {
        gbScene1 = false;
        gbScene2 = true;
    }

    //fade
    if(bFade == false)
    {
        if(fadeAlpha >= 0.0f)
            fadeAlpha -= 0.01f;    
    }
    else
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.01f;
    }
}

void Scene2(void)
{    
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 1.0f;
    static bool bFade = false;

    static GLfloat humanTrans = 0.01f;
    static GLfloat xEye = -3.0f;

    //code
    glBindTexture(GL_TEXTURE_2D, night_texture);
    RenderSky();

    glPushMatrix();
        //camera    
        gluLookAt(xEye, -0.2f, -10.0f + humanTrans, 
                  -1.5f, -0.2f, -10.0f + humanTrans,
                  0.0f, 1.0f, 0.0f);

        //lights
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

        //scene
        glCallList(groundList);   

        //draw shadow first
        for(int i = 0; i < MAX_HUMANOIDS; i++)
        {
            if(((i % 2) == 0) && 
               humanoidLooks[i].z <= 8.0f && 
               humanoidLooks[i].z >= -5.0f)
            {
                glPushMatrix();
                    glTranslatef(humanoidLooks[i].x, 0.0f, humanoidLooks[i].z);
                    if((i % 2) == 0)
                        glTranslatef(0.0f, 0.0f, -humanTrans);
                    else
                        glTranslatef(0.0f, 0.0f, humanTrans);

                    glCallList(shadowList);
                glPopMatrix();
            }
        }

        //shadow for jack
        glPushMatrix();
            glTranslatef(-1.5f, 0.0f, humanTrans - 10.0f);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            jackLooks.bShadow = true;

            glMultMatrixf(shadowMat);
            RenderHumanoid(&jackProp, jackLooks);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
        glPopMatrix();

        glCallList(treeList);

        //draw humanoids
        for(int i = 0; i < MAX_HUMANOIDS; i++)
        {
            if(((i % 2) == 0) && 
               humanoidLooks[i].z <= 8.0f && 
               humanoidLooks[i].z >= -5.0f)
            {
                humanoidLooks[i].bShadow = false;
                glPushMatrix();
                    glTranslatef(humanoidLooks[i].x, 0.0f, humanoidLooks[i].z);
                    if((i % 2) == 0)
                        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

                    glTranslatef(0.0f, 0.0f, humanTrans);
                    RenderHumanoid(&humanoidProp, humanoidLooks[i]);
                glPopMatrix();
            }
        }   

        //jack
        glPushMatrix();
            glTranslatef(-1.5f, 0.0f, humanTrans - 10.0f);

            jackLooks.bShadow = false;
            RenderHumanoid(&jackProp, jackLooks);
        glPopMatrix();
    glPopMatrix();

    //fade 
    Fade(fadeAlpha);

    //update 
    UpdateHumanoid(&humanoidProp);
    UpdateHumanoid(&jackProp);

    if(humanTrans <= 20.0f)
        humanTrans += 0.007f;

    if(xEye >= -8.0f)
        xEye -= 0.001f;

    if(humanTrans >= 12.5f)
        bFade = true;

    if(humanTrans >= 13.5f)
    {
        gbScene2 = false;
        gbScene3 = true;
    }

    //fade
    if(bFade == false)
    {
        if(fadeAlpha >= 0.0f)
            fadeAlpha -= 0.01f;    
    }
    else
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.02f;
    }
}

void Scene3(void)
{    
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 1.0f;

    static GLfloat yEye = -0.2f;
    static GLfloat zJack = -15.0f;
    static GLfloat xBird = -22.0f;
    static GLfloat yBird = 3.0f;
    static GLfloat zBird = -2.0f;
    static GLfloat xInc = 0.018f;
    static GLfloat birdRot = 0.0f;
    static float angle = 0.0f;

    static bool bRot = false;
    static bool headRot = false;

    //code
    glBindTexture(GL_TEXTURE_2D, night_texture);
    RenderSky();

    glPushMatrix();
        //camera    
        gluLookAt(3.0f, yEye, zJack, 
                  0.0f, yEye, zJack,
                  0.0f, 1.0f, 0.0f);

        //lights
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

        //scene
        glCallList(groundList);  

        //shadow for jack
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, zJack);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            jackLooks.bShadow = true;
    
            glMultMatrixf(shadowMat);
            RenderHumanoid(&jackProp, jackLooks);
        
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
        glPopMatrix();

        glCallList(treeList);

        //jack
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, zJack);
            glRotatef(birdRot, 0.0f, 1.0f, 0.0f);
            jackLooks.bShadow = false;
            RenderHumanoid(&jackProp, jackLooks);
        glPopMatrix();

        glPushMatrix();
            glTranslatef(xBird, yBird, zBird);
            glRotatef(birdRot, 0.0f, 1.0f, 0.0f);
            glScalef(0.1f, 0.1f, 0.1f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            RenderBird(bird_texture, feather_texture);
        glPopMatrix();
    glPopMatrix();

    //fade 
    Fade(fadeAlpha);

    //update 
    UpdateHumanoid(&jackProp);

    if(zJack <= -2.94f)
        zJack += 0.01f;
    else
        jackProp.bWalk = false;
    
    if(zJack >= -5.0f && zJack <= -3.0f)
        jackProp.headAngle -= 0.2f;

    if(bRot == false)
    {
        xBird += xInc;
        if(xBird >= 0.0f)
        {
            bRot = true;
            xInc = -xInc;
        }

        if(yBird >= 0.5f)
            yBird -= 0.0028f;
    }
    else
    {
        xBird = sin(angle);
        zBird = cos(angle) - 3.0f; 

        angle += 0.01f;
        if(angle >= M_PI)
        {
            bRot = false;
            headRot = true;
        }
        birdRot = angle * 180.0f / M_PI;
    }

    if(headRot == true)
    {    
        if(jackProp.headAngle >= 55.0f)
            jackProp.headAngle -= 0.03f;

        if(yEye <= 1.8f)
            yEye += 0.001f;
        else
        {
            gbScene3 = false;
            gbScene4 = true;
        }
    }
    
    //fade
    if(fadeAlpha >= 0.0f)
        fadeAlpha -= 0.01f;    
    
}

void Scene4(void)
{
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 0.0f;
    static bool bFade = false;

    static GLfloat yEye = 1.8f;
    static GLfloat xJack = 0.0f;
    static GLfloat xBird = -15.0f;
    static GLfloat yBird = 1.0f;
    static GLfloat zBird = -4.0f;
    static GLfloat angle = M_PI;
    static GLfloat birdRot = 0.0f;
    static bool bWalk = false;

    //code
    glBindTexture(GL_TEXTURE_2D, night_texture);
    RenderSky();

    glPushMatrix();
        //camera   
        gluLookAt(xJack + 3.0f, yEye, -2.94f, 
                  xJack, yEye, -2.94f,
                  0.0f, 1.0f, 0.0f);

        //lights
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

        //scene
        glCallList(groundList); 

        //shadow for jack
        glPushMatrix();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            jackLooks.bShadow = true;
            jackLooks.bChains = false;
            jackProp.bWalk = bWalk;
            jackProp.headAngle = 90.0f;

            glPushMatrix();
                glTranslatef(xJack, 0.0f, -3.3f);
                glMultMatrixf(shadowMat);
                glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
                RenderHumanoid(&jackProp, jackLooks);
            glPopMatrix();

            glPushMatrix();
                glMultMatrixf(shadowMat);
                glBegin(GL_LINES);
                    glColor3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(0.0f, -0.38f, -2.94f);
                    glVertex3f(0.0f, 50.0f, -2.94f);

                    glVertex3f(0.1f, -0.38f, -2.95f);
                    glVertex3f(0.0f, 50.0f, 1.9f);

                    glVertex3f(-0.1f, -0.38f, -2.93f);
                    glVertex3f(0.0f, 50.0f, -7.2f);
                 glEnd();
            glPopMatrix();

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
        glPopMatrix();

        glCallList(treeList); 

        //jack
        glPushMatrix();
            glTranslatef(xJack, 0.0f, -3.3f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            jackLooks.bShadow = false;
            
            RenderHumanoid(&jackProp, jackLooks);
        glPopMatrix();

        //steady ropes
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.62109f, 0.46875f, 0.375f);

        glBegin(GL_LINES);
            glVertex3f(0.0f, -0.38f, -2.94f);
            glVertex3f(0.0f, 50.0f, -2.94f);

            glVertex3f(0.1f, -0.38f, -2.95f);
            glVertex3f(0.0f, 50.0f, 1.9f);

            glVertex3f(-0.1f, -0.38f, -2.93f);
            glVertex3f(0.0f, 50.0f, -7.2f);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glPushMatrix();
            glTranslatef(xBird, yBird, zBird);
            glRotatef(birdRot, 0.0f, 1.0f, 0.0f);
            glScalef(0.1f, 0.1f, 0.1f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            RenderBird(bird_texture, feather_texture);
        glPopMatrix();
    glPopMatrix();

    //fade 
    Fade(fadeAlpha);

    //update
    UpdateHumanoid(&jackProp);

    if(yEye >= 0.0f)
        yEye -= 0.001f;
    else
        bWalk = true;
    
    if(bWalk == true)
    {
        if(xJack >= -13.0f)
            xJack -= 0.01f;
    }

    if(xJack <= -11.0f)
        bFade = true;

    if(xJack <= -12.5f)
    {
        gbScene4 = false;
        gbScene5 = true;
    }

    //rotate bird around tree
    xBird = sin(angle) - 15.0f;
    zBird = cos(angle) - 4.0f; 

    angle += 0.01f;
    if(angle >= 2 * M_PI)
        angle = 0.0f;

    birdRot = angle * 180.0f / M_PI;

    //fade out
    if(bFade)
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.01f;
    }
}

void Scene5(void)
{    
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 1.0f;
    static bool bFade = false;

    static GLfloat xBird = 0.0f;
    static GLfloat zBird = 0.0f;
    static GLfloat angle = 0.0f;
    static GLfloat birdRot = 0.0f;
    static GLfloat xEye = -11.8f;
    static GLfloat yEye = 0.2f;
    static GLfloat zEye = -1.0f;

    //code
    glBindTexture(GL_TEXTURE_2D, night_texture);
    RenderSky();

    glPushMatrix();
        //camera   
        gluLookAt(xEye, yEye, zEye, 
                  -12.0f, 0.2f, -4.0f,
                  0.0f, 1.0f, 0.0f);

        //lights
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

        //scene
        glCallList(groundList); 

         //shadow for jack
        glPushMatrix();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            jackLooks.bShadow = true;
            jackLooks.bChains = true;
            jackProp.headAngle = 80.0f;

            glLineWidth(2.0f);
            glTranslatef(-11.8f, 0.0f, -2.0f);
            glMultMatrixf(shadowMat);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            RenderHumanoid(&jackProp, jackLooks);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
        glPopMatrix();

        glCallList(treeList);

        //jack
        glPushMatrix();
            glTranslatef(-11.8f, 0.0f, -2.0f);
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            jackLooks.bShadow = false;
            jackProp.bWalk = false;
            jackLooks.bChains = false;

            RenderHumanoid(&jackProp, jackLooks);
        glPopMatrix();

        //bird
        glPushMatrix();
            glTranslatef(xBird, 1.0f, zBird);
            glRotatef(birdRot, 0.0f, 1.0f, 0.0f);
            glScalef(0.1f, 0.1f, 0.1f);
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            RenderBird(bird_texture, feather_texture);
        glPopMatrix();
    glPopMatrix();

    //fade 
    Fade(fadeAlpha);

    //update
    UpdateHumanoid(&jackProp);

    //rotate bird around tree
    xBird = 0.6f * sin(angle) - 13.6f;
    zBird = 0.6f * cos(angle) - 2.0f; 

    angle += 0.01f;
    if(angle >= 2 * M_PI)
        angle = 0.0f;

    birdRot = angle * 180.0f / M_PI;

    //camera 
    if(zEye <= 5.6f)
        zEye += 0.002f;
    else
    {
        gbScene5 = false;
        gbEndCredits = true;
    }
    
    if(zEye >= 5.0f)
        bFade = true;

    if(xEye >= -16.0f)
        xEye -= 0.0008f;    

    if(yEye <= 1.8f)
        yEye += 0.0005f;

    //fade
    if(bFade == false)
    {
        if(fadeAlpha >= 0.0f)
            fadeAlpha -= 0.008f;    
    }
    else
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.01f;
    }
}

void EndCredits(void)
{
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static GLfloat fadeAlpha = 1.0f;
    static bool bFade = false;

    static GLfloat yTrans = -2.0f;
    static float timer = 0.0f;
    static bool bQuote = true;
    static bool bCredits = false;

    char *szDestiny = "\" It is not in the stars to hold our destiny but in ourselves. \"";
    char *szShakespeare =  "- William Shakespeare";
    
    char *szCredits = "Credits";

    char *szDirectedBy = "Directed by";
    char *szYashPatel = "YASH PATEL";
    char *szMusicBy = "Music by";
    char *szHansZimmer = "HANZ ZIMMER (tennessee)";

    char *szRTRBatch = "RTR Batch";
    char *szYear = "2020-21";
    char *szGroup = "Group Name";
    char *szRender = "RENDER";
    char *szGroupLeader = "Group Leader";
    char *szBharatMazire = "BHARAT MAZIRE";

    char *szRenderingTechnology = "Rendering Technology";
    char *szOpenGL = "OPENGL (Fixed Function Pipeline)";
    char *szPlatform = "Platform";
    char *szWindows = "WINDOWS";
    char *szAudioEditor = "Audio Editor";
    char *szAudacity = "AUDACITY";

    char *szReferences = "References";

    char *szRedbook = "OpenGL Programming Guide Third Edition";
    char *szMasonWoo = "- Mason Woo";
    char *szJackieNeider = "- Jackie Neider";
    char *szTomDavis = "- Tom Davis";
    char *szDaveShreiner = "- Dave Shreiner";
    char *szSuperbible = "OpenGL Superbible Third Edition";
    char *szRichardWright = "- Richard S. Wright Jr";
    char *szBenjaminLipchak = "- Benjamin Lipchak";

    //code
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -3.0f);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);

        if(bQuote)
        {
            glPushMatrix();
                glTranslatef(-1.1f, -0.0f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szDestiny), GL_UNSIGNED_BYTE, szDestiny);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.4f, -0.1f, 0.0f);
                glScalef(0.09f, 0.09f, 0.09f);

                glListBase(fontList);
                glCallLists(strlen(szShakespeare), GL_UNSIGNED_BYTE, szShakespeare);
            glPopMatrix();
        }

        if(bCredits)
        {
            glTranslatef(0.0f, yTrans, 0.0f);
            glPushMatrix();
                glTranslatef(-0.2f, 0.2f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szCredits), GL_UNSIGNED_BYTE, szCredits);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, 0.0f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szDirectedBy), GL_UNSIGNED_BYTE, szDirectedBy);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, 0.0f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szYashPatel), GL_UNSIGNED_BYTE, szYashPatel);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -0.12f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szMusicBy), GL_UNSIGNED_BYTE, szMusicBy);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -0.12f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szHansZimmer), GL_UNSIGNED_BYTE, szHansZimmer);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -0.36f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szRTRBatch), GL_UNSIGNED_BYTE, szRTRBatch);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -0.36f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szYear), GL_UNSIGNED_BYTE, szYear);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -0.48f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szGroup), GL_UNSIGNED_BYTE, szGroup);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -0.48f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szRender), GL_UNSIGNED_BYTE, szRender);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -0.6f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szGroupLeader), GL_UNSIGNED_BYTE, szGroupLeader);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -0.6f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szBharatMazire), GL_UNSIGNED_BYTE, szBharatMazire);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -0.84f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szRenderingTechnology), GL_UNSIGNED_BYTE, szRenderingTechnology);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -0.84f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szOpenGL), GL_UNSIGNED_BYTE, szOpenGL);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -0.96f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szPlatform), GL_UNSIGNED_BYTE, szPlatform);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -0.96f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szWindows), GL_UNSIGNED_BYTE, szWindows);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.9f, -1.08f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szAudioEditor), GL_UNSIGNED_BYTE, szAudioEditor);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(0.2f, -1.08f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szAudacity), GL_UNSIGNED_BYTE, szAudacity);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -1.28f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szReferences), GL_UNSIGNED_BYTE, szReferences);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.8f, -1.48f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szRedbook), GL_UNSIGNED_BYTE, szRedbook);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -1.6f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szMasonWoo), GL_UNSIGNED_BYTE, szMasonWoo);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -1.6f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szMasonWoo), GL_UNSIGNED_BYTE, szMasonWoo);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -1.72f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szJackieNeider), GL_UNSIGNED_BYTE, szJackieNeider);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -1.84f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szTomDavis), GL_UNSIGNED_BYTE, szTomDavis);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -1.96f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szDaveShreiner), GL_UNSIGNED_BYTE, szDaveShreiner);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.6f, -2.2f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szSuperbible), GL_UNSIGNED_BYTE, szSuperbible);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -2.32f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szRichardWright), GL_UNSIGNED_BYTE, szRichardWright);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.2f, -2.44f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);

                glListBase(fontList);
                glCallLists(strlen(szBenjaminLipchak), GL_UNSIGNED_BYTE, szBenjaminLipchak);
            glPopMatrix();
        }
    glPopMatrix();

    //fade
    if(bQuote)
        Fade(fadeAlpha);
    
    //update 
    if(timer <= 4.0f)
        timer += 0.01f;
    else
        bFade = true;

    if(bCredits)
    {
        if(yTrans <= 4.0f)
            yTrans += 0.005f;
        else
        {
            gbEndCredits = false;
            gbSpecialCredits = true;
        }
        
    }

    //fade
    if(bFade == false)
    {
        if(fadeAlpha >= 0.0f)
            fadeAlpha -= 0.01f;    
    }
    else
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.01f;
        else
        {
            bQuote = false;
            bCredits = true;
        }
    }
}

void SpecialCredits(void)
{
    //function declaration
    void Fade(GLfloat alpha);

    //variable declaration
    static float timer = 0.0f;
    static GLfloat fadeAlpha = 1.0f;
    static bool bFade = false;
    static bool bFrame1 = true;
    static bool bFrame2 = false;

    char *szSpecialThanksTo = "Special Thanks To";
    char *szMaam = "DR. RAMA GOKHALE Ma'am";
    char *szScreenplayAndCinematography = "For Screenplay And Cinematography Guidance";
    char *szDedicatedTo = "Dedicated To";
    char *szSir = "DR. VIJAY GOKHALE Sir";

    //code
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
        glTranslatef(0.0f, 0.0f, -3.0f);
        glColor3f(1.0f, 1.0f, 1.0f);

        if(bFrame1)
        {
            glPushMatrix();
                glTranslatef(-0.4f, 0.1f, 0.0f);
                glScalef(0.09f, 0.09f, 0.09f);
                glListBase(fontList);
                glCallLists(strlen(szSpecialThanksTo), GL_UNSIGNED_BYTE, szSpecialThanksTo);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.6f, 0.0f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);
                glListBase(fontList);
                glCallLists(strlen(szMaam), GL_UNSIGNED_BYTE, szMaam);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.8f, -0.1f, 0.0f);
                glScalef(0.08f, 0.08f, 0.08f);
                glListBase(fontList);
                glCallLists(strlen(szScreenplayAndCinematography), GL_UNSIGNED_BYTE, szScreenplayAndCinematography);
            glPopMatrix();
        }

        if(bFrame2)
        {
            glPushMatrix();
                glTranslatef(-0.3f, 0.15f, 0.0f);
                glScalef(0.1f, 0.1f, 0.1f);
                glListBase(fontList);
                glCallLists(strlen(szDedicatedTo), GL_UNSIGNED_BYTE, szDedicatedTo);
            glPopMatrix();

            glPushMatrix();
                glTranslatef(-0.65f, 0.0f, 0.0f);
                glScalef(0.13f, 0.13f, 0.13f);
                glListBase(fontList);
                glCallLists(strlen(szSir), GL_UNSIGNED_BYTE, szSir);
            glPopMatrix();
        }
    glPopMatrix();
    
    Fade(fadeAlpha);

    //update
    //fade
    if(bFade == false)
    {
        if(fadeAlpha >= 0.0f)
            fadeAlpha -= 0.01f;    
    }
    else
    {
        if(fadeAlpha <= 1.0f)
            fadeAlpha += 0.01f;
        else if(bFrame1 == true)
        {
            bFade = false;
            bFrame1 = false;
            bFrame2 = true;
        }
    }
    
    if(timer <= 3.5f)
        timer += 0.01f;
    else 
    {
        bFade = true;
        timer = 0.0f;
    }
}

void Fade(GLfloat alpha)
{
    //code
    glBegin(GL_QUADS);
        glColor4f(0.0f, 0.0f, 0.0f, alpha);
        glVertex3f(1.0f, 1.0f, -0.2f);
        glVertex3f(-1.0f, 1.0f, -0.2f);
        glVertex3f(-1.0f, -1.0f, -0.2f);
        glVertex3f(1.0f, -1.0f, -0.2f);
    glEnd();
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

    //delete display lists
    glDeleteLists(groundList, 1);
    glDeleteLists(treeList, 1);
    glDeleteLists(fontList, 128);
    glDeleteLists(shadowList, 1);

    //delete textures
    glDeleteTextures(1, &ground_texture);
    glDeleteTextures(1, &day_texture);
    glDeleteTextures(1, &night_texture);
    glDeleteTextures(1, &bird_texture);
    glDeleteTextures(1, &feather_texture);

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
