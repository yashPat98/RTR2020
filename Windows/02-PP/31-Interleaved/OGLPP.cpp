//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include "vmath.h"                 //Maths header
#include "RESOURCES.h"             //Resources header

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

//symbolic constants
#define WIN_WIDTH  800             //initial width of window  
#define WIN_HEIGHT 600             //initial height of window

#define VK_F       0x46            //virtual key code of F key
#define VK_f       0x60            //virtual key code of f key

//namespaces
using namespace vmath;

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

//callback procedure declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//global variables
HWND   ghwnd  = NULL;              //handle to a window
HDC    ghdc   = NULL;              //handle to a device context
HGLRC  ghrc   = NULL;              //handle to a rendering context

DWORD dwStyle = NULL;              //window style
WINDOWPLACEMENT wpPrev;            //structure for holding previous window position

bool gbActiveWindow = false;       //flag indicating whether window is active or not
bool gbFullscreen = false;         //flag indicating whether window is fullscreen or not

FILE*  gpFile = NULL;              //log file

GLuint vertexShaderObject;         //handle to vertex shader object
GLuint fragmentShaderObject;       //handle to fragment shader object
GLuint shaderProgramObject;        //handle to shader program object

GLuint vao;
GLuint vbo;

GLuint modelviewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint LKeyPressedUniform;
GLuint LdUniform;
GLuint lightPositionUniform;
GLuint diffuseTextureUniform;
GLuint TKeyPressedUniform;

GLuint marble_texture;

mat4 perspectiveProjectionMatrix; 
GLfloat cube_rotation_angle = 0.0f; 

bool bAnimate;
bool bLight;
bool bTexture;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                 //initialize OpenGL state machine
    void Display(void);                                    //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                   //structure holding window class attributes
    MSG msg;                                               //structure holding message attributes
    HWND hwnd;                                             //handle to a window
    TCHAR szAppName[] = TEXT("OpenGL : Interleaved");       //name of window class

    int cxScreen, cyScreen;                                //screen width and height for centering window
    int init_x, init_y;                                    //top-left coordinates of centered window
    bool bDone = false;                                    //flag indicating whether or not to exit from game loop

    //code
    //create/open  'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Failed to open log.txt file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "----- Program Started Successfully -----\n\n");
    }
    
    //initialization of WNDCLASSEX
    wndclass.cbSize         = sizeof(WNDCLASSEX);                            //size of structure
    wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;            //window style
    wndclass.lpfnWndProc    = WndProc;                                       //address of callback procedure
    wndclass.cbClsExtra     = 0;                                             //extra class bytes
    wndclass.cbWndExtra     = 0;                                             //extra window bytes
    wndclass.hInstance      = hInstance;                                     //handle to a program
    wndclass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));  //handle to an icon
    wndclass.hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);        //handle to a cursor
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);           //handle to a background brush
    wndclass.lpszClassName  = szAppName;                                     //name of a custom class
    wndclass.lpszMenuName   = NULL;                                          //name of a custom menu
    wndclass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));  //handle to a small icon

    //register above class
    RegisterClassEx(&wndclass);

    //get screen width and height
    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    //calculate top-left coordinates for a centered window
    init_x = (cxScreen / 2) - (WIN_WIDTH / 2);
    init_y = (cyScreen / 2) - (WIN_HEIGHT / 2);

    //create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,                //extended window style          
            szAppName,                                    //class name
            szAppName,                                    //window caption
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |       //window style
            WS_CLIPSIBLINGS | WS_VISIBLE,   
            init_x,                                       //X-coordinate of top left corner of window 
            init_y,                                       //Y-coordinate of top left corner of window
            WIN_WIDTH,                                    //initial window width                 
            WIN_HEIGHT,                                   //initial window height
            (HWND)NULL,                                   //handle to a parent window  : NULL desktop
            (HMENU)NULL,                                  //handle to a menu : NULL no menu
            hInstance,                                    //handle to a program instance
            (LPVOID)NULL);                                //data to be sent to window callback : NULL no data to send      

    //store handle to a window in global handle
    ghwnd = hwnd;                                         

    //initialize OpenGL rendering context
    Initialize();
    
    ShowWindow(hwnd, iCmdShow);                 //set specified window's show state
    SetForegroundWindow(hwnd);                  //brings the thread that created the specified window to foreground
    SetFocus(hwnd);                             //set the keyboard focus to specified window 

    //game loop
    while(bDone == false)
    {   
        //1 : pointer to structure for window message
        //2 : handle to window : NULL do not process child window's messages 
        //3 : message filter min range : 0 no range filtering
        //4 : message filter max range : 0 no range filtering
        //5 : remove message from queue after processing from PeekMessage
        if(PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)           //if current message is WM_QUIT then exit from game loop
            {
                bDone = true;
            }
            else
            {
                TranslateMessage(&msg);          //translate virtual-key message into character message
                DispatchMessage(&msg);           //dispatch message  to window procedure
            }
        }
        else
        {
            if(gbActiveWindow == true)           //if window has keyboard focus 
            {
                Display();                       //render the scene
            }
        }
    }

    return ((int)msg.wParam);                    //exit code given by PostQuitMessage 
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //function declarations
    void ToggleFullscreen(void);                 //toggle window between fullscreen and previous position 
    void Resize(int, int);                       //handle window resize event
    void UnInitialize(void);                     //release resources  

    //code
    switch(iMsg)
    {
        case WM_SETFOCUS:                        //event : window has keyboard focus
            gbActiveWindow = true;
            break;
        
        case WM_KILLFOCUS:                       //event : window dosen't have keyboard focus
            gbActiveWindow = false;
            break;

        case WM_ERASEBKGND:                      //event : window background must be erased 
            return (0);                          //dont let DefWindowProc handle this event
        
        case WM_SIZE:                            //event : window is resized
            Resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:                         //event : a key has been pressed
            switch(wParam)
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case VK_F:
                case VK_f:
                    ToggleFullscreen();
                    break;
                
                default:
                    break;
            }
            break;
        
        case WM_CHAR:
            switch(wParam)
            {
                case 'A':
                case 'a':
                    if(bAnimate)
                    {
                        bAnimate = false;
                    }
                    else
                    {
                        bAnimate = true;
                    }
                    break;

                case 'L':
                case 'l':
                    if(bLight)
                    {
                        bLight = false;
                    }
                    else
                    {
                        bLight = true;
                    }
                    break;

                case 'T':
                case 't':
                    if(bTexture)
                    {
                        bTexture = false;
                    }
                    else
                    {
                        bTexture = true;
                    }
                    break;
                
                default:
                    break;
            }
            break;

        case WM_CLOSE:                           //event : window is closed from sysmenu or close button
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            UnInitialize();
            PostQuitMessage(0);
            break;
        
        default:
            break;
    }

    //call default window procedure for unhandled messages
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
    //variable declarations
    MONITORINFO mi = { sizeof(MONITORINFO) };            //structure holding monitor information

    //code
    if(gbFullscreen == false)                            //if screen is not in fulscreen mode 
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);       //get window style
        if(dwStyle & WS_OVERLAPPEDWINDOW)                //if current window style has WS_OVERLAPPEDWINDOW
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                // if wpPrev is successfully filled with current window placement
                // and mi is successfully filled with primary monitor info then
                // 1 -> Remove WS_OVERLAPPEDWINDOW style
                // 2 -> Set window position by aligning left-top corner of window 
                //     to left-top corner of monitor and setting width and height 
                //     to monitor's width and height (effectively making window 
                //     fullscreen)
                // SWP_NOZORDER : Don't change Z-order
                // SWP_FRAMECHANGED: Forces recalculation of New Client area (WM_NCCALCSIZE)
                SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd,                                     //     top 
                    HWND_TOP,                                           //left +--------------+ right
                    mi.rcMonitor.left,                                  //     |              |
                    mi.rcMonitor.top,                                   //     |              |
                    mi.rcMonitor.right - mi.rcMonitor.left,             //     |              |
                    mi.rcMonitor.bottom - mi.rcMonitor.top,             //     |              |
                    SWP_NOZORDER | SWP_FRAMECHANGED);                   //     +--------------+
            }                                                           //     bottom
        }

        ShowCursor(false);                                 //hide the cursor
        gbFullscreen = true;                          
    }
    else                                                   //if screen is in fullscreen mode
    {
        // Toggle the window to previously saved dimension
        // 1 -> Add WS_OVERLAPPEDWINDOW to window style 
        // 2 -> Set window placement to stored previous placement
        // 3 -> Force the effects of SetWindowPlacement by call to 
        //      SetWindowPos with
        // SWP_NOMOVE : Don't change left top position of window 
        //              i.e ignore third and forth parameters
        // SWP_NOSIZE : Don't change dimensions of window
        //              i.e ignore fifth and sixth parameters
        // SWP_NOZORDER : Don't change Z-order of the window and
        //              its child windows
        // SWP_NOOWNERZORDER : Don't change Z-order of owner of the 
        //              window (reffered by ghwnd)
        // SWP_FRAMECHANGED : Forces recalculation of New Client area (WM_NCCALCSIZE)
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0, 
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        
        ShowCursor(true);            //show cursor
        gbFullscreen = false;
    }
}

void Initialize(void)
{
    //function declarations
    void Resize(int, int);          //warm-up call
    void UnInitialize(void);        //release resources
    bool loadGLTexture(GLuint *texture, TCHAR ResourceID[]);

    //variable declarations
    PIXELFORMATDESCRIPTOR pfd;      //structure describing the pixel format
    int iPixelFormatIndex;          //index of the pixel format structure in HDC

    //code
    //zero out the memory
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR)); 

    //initialization of PIXELFORMATDESCRIPTOR
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);                                //size of structure
    pfd.nVersion    = 1;                                                            //version information
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW| PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;    //pixel format properties
    pfd.iPixelType  = PFD_TYPE_RGBA;                                                //type of pixel format to chosen
    pfd.cColorBits  = 32;                                                           //color depth in bits (32 = True Color)
    pfd.cRedBits    = 8;                                                            //red color bits
    pfd.cGreenBits  = 8;                                                            //green color bits
    pfd.cBlueBits   = 8;                                                            //blue color bits
    pfd.cAlphaBits  = 8;                                                            //alpha bits
    pfd.cDepthBits  = 32;                                                           //depth bits

    //obtain a device context
    ghdc = GetDC(ghwnd);                    

    //choose required pixel format from device context
    //which matches pfd structure and get the index of 
    //that pixel format (1 based index)
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        fprintf(gpFile, "ChoosePixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set the current pixel format of the device context (ghdc) to
    //pixel format specified by index
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //create rendering context 
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set rendering context as current context
    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed.\n");
        DestroyWindow(ghwnd);
    }

    //initialize glew (enable extensions)
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK)
    {
        fprintf(gpFile, "glewInit() failed.\n");
        DestroyWindow(ghwnd);
    }

    //opengl related log
    fprintf(gpFile, "OpenGL Information\n");
    fprintf(gpFile, "OpenGL Vendor     : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer   : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version    : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version      : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    //opengl enabled extensions
    GLint numExt;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    fprintf(gpFile, "OpenGL Extensions : \n");
    for(int i = 0; i < numExt; i++)
    {
        fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
    }

    //setup render scene

    //--- Vertex Shader ---

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                                 \
        "\n"                                                                                \
        "in vec3 vPosition;"                                                                \
        "in vec3 vColor;"                                                                   \
        "in vec3 vNormal;"                                                                  \
        "in vec2 vTexCoord;"                                                                \

        "uniform mat4 u_modelviewMatrix;"                                                   \
        "uniform mat4 u_projectionMatrix;"                                                  \
        "uniform vec3 u_Ld;"                                                                \
        "uniform vec4 u_lightPosition;"                                                     \
        "uniform int u_LKeyPressed;"                                                        \
        
        "out vec3 diffuse_light;"                                                           \
        "out vec2 out_texcoord;"                                                            \

        "void main(void)"                                                                   \
        "{"                                                                                 \
        "   if(u_LKeyPressed == 1)"                                                         \
        "   {"                                                                              \
        "       vec4 eye_coords = u_modelviewMatrix * vec4(vPosition, 1.0f);"               \
        "       mat3 normal_matrix = mat3(transpose(inverse(u_modelviewMatrix)));"          \
        "       vec3 tnorm = normalize(normal_matrix * vNormal);"                           \
        "       vec3 s = normalize(vec3(u_lightPosition - eye_coords));"                    \
        "       diffuse_light = u_Ld * vColor * max(dot(s, tnorm), 0.0);"                   \
        "   }"                                                                              \
        
        "   out_texcoord = vTexCoord;"                                                      \
        "   gl_Position = u_projectionMatrix * u_modelviewMatrix * vec4(vPosition, 1.0f);"  \
        "}";

    //provide source code to shader object
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //--- Fragment Shader ---

    //create shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                 \
        "\n"                                                                \
        
        "in vec3 diffuse_light;"                                            \
        "in vec2 out_texcoord;"                                             \

        "uniform sampler2D u_diffuseTexture;"                               \
        "uniform int u_LKeyPressed;"                                        \
        "uniform int u_TKeyPressed;"                                        \
        
        "out vec4 FragColor;"                                               \
        
        "void main(void)"                                                   \
        "{"                                                                 \
        "   vec4 color;"                                                    \
        "   if(u_LKeyPressed == 1)"                                         \
        "   {"                                                              \
        "       color = vec4(diffuse_light, 1.0f);"                         \
        "   }"                                                              \
        "   else"                                                           \
        "   {"                                                              \
        "       color = vec4(1.0f, 1.0f, 1.0f, 1.0f);"                      \
        "   }"                                                              \

        "   if(u_TKeyPressed == 1)"                                         \
        "   {"                                                              \
        "       color *= texture(u_diffuseTexture, out_texcoord);"           \
        "   }"                                                              \

        "   FragColor = color;"                                             \
        "}";

    //provide source code to shader object 
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //--- Shader Program ---

    //create shader program
    shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(shaderProgramObject, vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    //binding of shader program object with vertex shader attributes
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");

    //link shader program 
    glLinkProgram(shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get uniform locations
    modelviewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelviewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");
    
    LdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
    diffuseTextureUniform = glGetUniformLocation(shaderProgramObject, "u_diffuseTexture");

    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");
    LKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");
    TKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_TKeyPressed");

    //cube data
    const GLfloat cubePCNT[] = 
    {
        //near 
        +1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +1.0f, +1.0f,   
        -1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +1.0f,
        -1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f,
        +1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f, +1.0f,    +1.0f, +0.0f,

        //right
        +1.0f, +1.0f, -1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +1.0f, +1.0f,
        +1.0f, +1.0f, +1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +1.0f,
        +1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +0.0f, +0.0f,
        +1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f, +0.0f,    +1.0f, +0.0f,

        //far
        -1.0f, +1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +1.0f, +1.0f,
        +1.0f, +1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +0.0f, +1.0f,
        +1.0f, -1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +0.0f, +0.0f,
        -1.0f, -1.0f, -1.0f,    +0.0f, +0.0f, +1.0f,    +0.0f, +0.0f, -1.0f,    +1.0f, +0.0f,

        //left
        -1.0f, +1.0f, -1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +1.0f, +1.0f,
        -1.0f, +1.0f, +1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +0.0f, +1.0f,
        -1.0f, -1.0f, +1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +0.0f, +0.0f,
        -1.0f, -1.0f, -1.0f,    +1.0f, +0.0f, +1.0f,    -1.0f, +0.0f, +0.0f,    +1.0f, +0.0f,

        //top
        +1.0f, +1.0f, -1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +1.0f,
        -1.0f, +1.0f, -1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +0.0f, +1.0f,
        -1.0f, +1.0f, +1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +0.0f, +0.0f,
        +1.0f, +1.0f, +1.0f,    +1.0f, +1.0f, +0.0f,    +0.0f, +1.0f, +0.0f,    +1.0f, +0.0f,

        //bottom
        -1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +1.0f, +1.0f,
        +1.0f, -1.0f, -1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +0.0f, +1.0f,
        +1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +0.0f, +0.0f,
        -1.0f, -1.0f, +1.0f,    +0.0f, +1.0f, +1.0f,    +0.0f, -1.0f, +0.0f,    +1.0f, +0.0f,
    };

    //setup vao and vbo 
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, 24 * 11 * sizeof(GLfloat), cubePCNT, GL_STATIC_DRAW);
            //position
            glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
            //color
            glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
            //normal
            glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
            glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
            //texcoord
            glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
            glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //smooth shading  
    glShadeModel(GL_SMOOTH);                  

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();

    //load textures
    loadGLTexture(&marble_texture, MAKEINTRESOURCE(MARBLE_BITMAP));

    bAnimate = false;
    bLight = false;

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

bool loadGLTexture(GLuint *texture, TCHAR ResourceID[])
{
    //variable declarations
    bool bResult = false;
    HBITMAP hBitmap = NULL;
    BITMAP bmp;

    //code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if(hBitmap)
    {
        bResult = true;
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        //set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        //push the data to texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
        glGenerateMipmap(GL_TEXTURE_2D);

        //free resource
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    return (bResult);
}

void Resize(int width, int height)
{
    //code
    //if current height is 0 set 1 to avoid 
    //divide by 0 error 
    if(height == 0)
        height = 1;

    //set viewport transformation
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void Display(void)
{
    //variable declarations
    mat4 modelViewMatrix;
    mat4 modelViewProjectionMatrix;
    mat4 translateMatrix;
    mat4 rotationMatrix_x;
    mat4 rotationMatrix_y;
    mat4 rotationMatrix_z;
    mat4 scaleMatrix;

    GLfloat lightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);
        if(bLight == true)
        {
            glUniform1i(LKeyPressedUniform, 1);
            glUniform3f(LdUniform, 1.0f, 1.0f, 1.0f);
            glUniform4fv(lightPositionUniform, 1, (GLfloat*)lightPosition);
        }
        else
        {
            glUniform1i(LKeyPressedUniform, 0);
        }

        if(bTexture == true)
        {
            glUniform1i(TKeyPressedUniform, 1);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, marble_texture);
            glUniform1i(diffuseTextureUniform, 0);
        }
        else
        {
            glUniform1i(TKeyPressedUniform, 0);
        }

        //set modelview, modelviewprojection and translate matrix to identity
        modelViewMatrix = mat4::identity();
        modelViewProjectionMatrix = mat4::identity();
        translateMatrix = mat4::identity();
        rotationMatrix_x = mat4::identity();
        rotationMatrix_y = mat4::identity();
        rotationMatrix_z = mat4::identity();
        scaleMatrix = mat4::identity();

        //translate modelview matrix
        translateMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
        scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);
        rotationMatrix_x = vmath::rotate(cube_rotation_angle, 1.0f, 0.0f, 0.0f);
        rotationMatrix_y = vmath::rotate(cube_rotation_angle, 0.0f, 1.0f, 0.0f);
        rotationMatrix_z = vmath::rotate(cube_rotation_angle, 0.0f, 0.0f, 1.0f);
        modelViewMatrix = translateMatrix * scaleMatrix * rotationMatrix_x * rotationMatrix_y * rotationMatrix_z;

        //pass modelview and projection matrices to vertex shader
        glUniformMatrix4fv(modelviewMatrixUniform, 1, GL_FALSE, modelViewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        //bind vao for square
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    //update   
    if(bAnimate)
    {
        cube_rotation_angle += 0.05f;
        if(cube_rotation_angle >= 360.0f)
            cube_rotation_angle = 0.0f;
    }

    SwapBuffers(ghdc);
}

void UnInitialize(void)
{
    //code
    //if window is in fullscreen mode toggle
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
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
    
        ShowCursor(true);
        gbFullscreen = false;
    }

    //release texture
    if(marble_texture)
    {
        glDeleteTextures(1, &marble_texture);
        marble_texture = 0;
    }

    //release vao and vbo for square
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if(vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    //safe shader cleanup
    if(shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(shaderProgramObject);
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
        glUseProgram(0);
    }

    //HGLRC : NULL means calling thread's current rendering context 
    //        is no longer current as well as it releases the device 
    //        context used by that rendering context
    //HDC : is ignored if HGLRC is passed as NULL
    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent((HDC)NULL, (HGLRC)NULL);
    }

    //delete rendering context 
    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = (HGLRC)NULL;
    }

    //release the device context
    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = (HDC)NULL;
    }

    //close the log file
    if(gpFile)
    {
        fprintf(gpFile, "\n----- Program Completed Successfully -----\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
