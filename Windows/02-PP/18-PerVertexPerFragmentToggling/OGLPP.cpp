//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //C header 
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include "vmath.h"                 //Maths header
#include "RESOURCES.h"             //Resources header
#include "Sphere.h"                //Sphere header

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "Sphere.lib")

//symbolic constants
#define WIN_WIDTH  800             //initial width of window  
#define WIN_HEIGHT 600             //initial height of window

#define VK_Q       0x51            //virtual key code of Q key
#define VK_q       0x75            //virtual key code of q key

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
HWND   ghwnd  = NULL;               //handle to a window
HDC    ghdc   = NULL;               //handle to a device context
HGLRC  ghrc   = NULL;               //handle to a rendering context

DWORD dwStyle = NULL;               //window style
WINDOWPLACEMENT wpPrev;             //structure for holding previous window position

bool gbActiveWindow = false;        //flag indicating whether window is active or not
bool gbFullscreen = false;          //flag indicating whether window is fullscreen or not

FILE*  gpFile = NULL;               //log file

GLuint v_vertexShaderObject;        //handle to vertex shader object
GLuint v_fragmentShaderObject;      //handle to fragment shader object
GLuint v_shaderProgramObject;       //handle to shader program object

GLuint f_vertexShaderObject;        //handle to vertex shader object
GLuint f_fragmentShaderObject;      //handle to fragment shader object
GLuint f_shaderProgramObject;       //handle to shader program object

GLuint vao_sphere;                  //handle to vertex array object for sphere
GLuint vbo_sphere_position;         //handle to vertex buffer object for vertices of sphere
GLuint vbo_sphere_normal;           //handle to vertex buffer object for normals of sphere
GLuint vbo_sphere_texcoord;         //handle to vertex buffer object for texcoords of sphere
GLuint vbo_sphere_elements;         //handle to vertex buffer object for indices 

GLuint v_modelMatrixUniform;
GLuint v_viewMatrixUniform;
GLuint v_perspectiveProjectionMatrixUniform;
GLuint v_LaUniform;
GLuint v_LdUniform;
GLuint v_LsUniform;
GLuint v_lightPositionUniform;
GLuint v_KaUniform;
GLuint v_KdUniform;
GLuint v_KsUniform;
GLuint v_materialShininessUniform;
GLuint v_LKeyPressedUniform;

GLuint f_modelMatrixUniform;
GLuint f_viewMatrixUniform;
GLuint f_perspectiveProjectionMatrixUniform;
GLuint f_LaUniform;
GLuint f_LdUniform;
GLuint f_LsUniform;
GLuint f_lightPositionUniform;
GLuint f_KaUniform;
GLuint f_KdUniform;
GLuint f_KsUniform;
GLuint f_materialShininessUniform;
GLuint f_LKeyPressedUniform;
      
mat4 perspectiveProjectionMatrix;  

vec4 La;
vec4 Ld;
vec4 Ls;
vec4 lightPosition;

vec4 Ka;
vec4 Kd;
vec4 Ks;
float materialShininess;

int LKeyPressed;
bool perVertex_perFragmentToggle;

GLfloat sphere_vertices[1146];
GLfloat sphere_normals[1146];
GLfloat sphere_texcoords[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                      //initialize OpenGL state machine
    void Display(void);                                         //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                                        //structure holding window class attributes
    MSG msg;                                                                    //structure holding message attributes
    HWND hwnd;                                                                  //handle to a window
    TCHAR szAppName[] = TEXT("OpenGL : Per Vertex - Per Fragment Toggling");    //name of window class

    int cxScreen, cyScreen;                                                     //screen width and height for centering window
    int init_x, init_y;                                                         //top-left coordinates of centered window
    bool bDone = false;                                                         //flag indicating whether or not to exit from game loop

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
                case VK_Q:
                case VK_q:
                    DestroyWindow(hwnd);
                    break;
                    
                case VK_ESCAPE:
                    ToggleFullscreen();
                    break;

                default:
                    break;
            }
            break;
        
        case WM_CHAR:
            switch(wParam)
            {
                case 'L':
                case 'l':
                    if(LKeyPressed == 0)
                    {
                        LKeyPressed = 1;
                    }
                    else
                    {
                        LKeyPressed = 0;
                    }
                    break;

                case 'V':
                case 'v':
                    perVertex_perFragmentToggle = false;
                    break;
                
                case 'F':
                case 'f':
                    perVertex_perFragmentToggle = true;
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

    //--- Per Vertex Lighting ---
    
    fprintf(gpFile, "\n***** Per Vertex Lighting *****\n");

    //vertex shader

    //create shader
    v_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* v_vertexShaderSourceCode = 
        "#version 450 core"                                                                                                     \
        "\n"                                                                                                                    \
        "in vec4 vPosition;"                                                                                                    \
        "in vec3 vNormal;"                                                                                                      \
        "uniform mat4 u_modelMatrix;"                                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                                           \
        "uniform vec3 u_La;"                                                                                                    \
        "uniform vec3 u_Ld;"                                                                                                    \
        "uniform vec3 u_Ls;"                                                                                                    \
        "uniform vec4 u_lightPosition;"                                                                                         \
        "uniform vec3 u_Ka;"                                                                                                    \
        "uniform vec3 u_Kd;"                                                                                                    \
        "uniform vec3 u_Ks;"                                                                                                    \
        "uniform float u_materialShininess;"                                                                                    \
        "uniform int u_LKeyPressed;"                                                                                            \
        "out vec3 phong_ads_light;"                                                                                             \
        "void main(void)"                                                                                                       \
        "{"                                                                                                                     \
        "   if(u_LKeyPressed == 1)"                                                                                             \
        "   {"                                                                                                                  \
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                                    \
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix)));"                                   \
        "       vec3 transformed_normal = normalize(normal_matrix * vNormal);"                                                  \
        "       vec3 light_direction = normalize(vec3(u_lightPosition - eye_coords));"                                          \
        "       vec3 reflection_vector = reflect(-light_direction, transformed_normal);"                                        \
        "       vec3 view_vector = normalize(-eye_coords.xyz);"                                                                 \
        "       vec3 ambient = u_La * u_Ka;"                                                                                    \
        "       vec3 diffuse = u_Ld * u_Kd * max(dot(light_direction, transformed_normal), 0.0f);"                              \
        "       vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, view_vector), 0.0f), u_materialShininess);"        \
        "       phong_ads_light = ambient + diffuse + specular;"                                                                \
        "   }"                                                                                                                  \
        "   else"                                                                                                               \
        "   {"                                                                                                                  \
        "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                      \
        "   }"                                                                                                                  \
        "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"                            \
        "}";

    //provide source code to shader object
    glShaderSource(v_vertexShaderObject, 1, (const GLchar**)&v_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(v_vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

    glGetShaderiv(v_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(v_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(v_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //fragment shader

    //create shader
    v_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* v_fragmentShaderSourceCode = 
        "#version 450 core"                                 \
        "\n"                                                \
        "in vec3 phong_ads_light;"                          \
        "out vec4 FragColor;"                               \
        "void main(void)"                                   \
        "{"                                                 \
        "   FragColor = vec4(phong_ads_light, 1.0f);"       \
        "}";

    //provide source code to shader object 
    glShaderSource(v_fragmentShaderObject, 1, (const GLchar**)&v_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(v_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(v_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(v_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(v_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //shader program 

    //create shader program
    v_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(v_shaderProgramObject, v_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(v_shaderProgramObject, v_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(v_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(v_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link shader program 
    glLinkProgram(v_shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(v_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(v_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(v_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get uniform locations
    v_modelMatrixUniform = glGetUniformLocation(v_shaderProgramObject, "u_modelMatrix");
    v_viewMatrixUniform = glGetUniformLocation(v_shaderProgramObject, "u_viewMatrix");
    v_perspectiveProjectionMatrixUniform = glGetUniformLocation(v_shaderProgramObject, "u_perspectiveProjectionMatrix");

    v_LaUniform = glGetUniformLocation(v_shaderProgramObject, "u_La");
    v_LdUniform = glGetUniformLocation(v_shaderProgramObject, "u_Ld");
    v_LsUniform = glGetUniformLocation(v_shaderProgramObject, "u_Ls");
    v_lightPositionUniform = glGetUniformLocation(v_shaderProgramObject, "u_lightPosition");

    v_KaUniform = glGetUniformLocation(v_shaderProgramObject, "u_Ka");
    v_KdUniform = glGetUniformLocation(v_shaderProgramObject, "u_Kd");
    v_KsUniform = glGetUniformLocation(v_shaderProgramObject, "u_Ks");
    v_materialShininessUniform = glGetUniformLocation(v_shaderProgramObject, "u_materialShininess");

    v_LKeyPressedUniform = glGetUniformLocation(v_shaderProgramObject, "u_LKeyPressed");

    //--- Per Fragment Lighting ---

    fprintf(gpFile, "\n***** Per Fragment Lighting *****\n");

    //vertex shader

    //create shader
    f_vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* f_vertexShaderSourceCode = 
        "#version 450 core"                                                                                     \
        "\n"                                                                                                    \
        "in vec4 vPosition;"                                                                                    \
        "in vec3 vNormal;"                                                                                      \
        "uniform mat4 u_modelMatrix;"                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \
        "uniform vec4 u_lightPosition;"
        "uniform int u_LKeyPressed;"                                                                            \
        "out vec3 transformed_normal;"                                                                          \
        "out vec3 light_direction;"                                                                             \
        "out vec3 view_vector;"                                                                                 \
        "void main(void)"                                                                                       \
        "{"                                                                                                     \
        "   if(u_LKeyPressed == 1)"                                                                             \
        "   {"                                                                                                  \
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                    \
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix))); "                  \
        "       transformed_normal = normal_matrix * vNormal;"                                                  \
        "       light_direction = vec3(u_lightPosition - eye_coords);"                                          \
        "       view_vector = -eye_coords.xyz;"                                                                 \
        "   }"                                                                                                  \
        "   gl_Position = u_perspectiveProjectionMatrix * u_viewMatrix * u_modelMatrix * vPosition;"            \
        "}";                                                                  

    //provide source code to shader object
    glShaderSource(f_vertexShaderObject, 1, (const GLchar**)&f_vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(f_vertexShaderObject);

    //shader compilation error checking
    glGetShaderiv(f_vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(f_vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(f_vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "\n----- Vertex Shader Compiled Successfully -----\n");

    //--- Fragment Shader ---

    //create shader
    f_fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* f_fragmentShaderSourceCode = 
        "#version 450 core"                                                                                                             \
        "\n"                                                                                                                            \
        "in vec3 transformed_normal;"                                                                                                   \
        "in vec3 light_direction;"                                                                                                      \
        "in vec3 view_vector;"                                                                                                          \
        "uniform vec3 u_La;"                                                                                                            \
        "uniform vec3 u_Ld;"                                                                                                            \
        "uniform vec3 u_Ls;"                                                                                                            \
        "uniform vec3 u_Ka;"                                                                                                            \
        "uniform vec3 u_Kd;"                                                                                                            \
        "uniform vec3 u_Ks;"                                                                                                            \
        "uniform float u_materialShininess;"                                                                                            \
        "uniform int u_LKeyPressed;"                                                                                                    \
        "out vec4 fragColor;"                                                                                                           \
        "void main(void)"                                                                                                               \
        "{"                                                                                                                             \
        "   vec3 phong_ads_light;"                                                                                                      \
        "   if(u_LKeyPressed == 1)"                                                                                                       \
        "   {"                                                                                                                          \
        "       vec3 normalized_transformed_normal = normalize(transformed_normal);"                                                    \
        "       vec3 normalized_view_vector = normalize(view_vector);"                                                                  \
        "       vec3 normalized_light_direction = normalize(light_direction);"                                                          \
        "       vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normal);"                          \
        "       vec3 ambient = u_La * u_Ka;"                                                                                            \
        "       vec3 diffuse = u_Ld * u_Kd  * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f);"               \
        "       vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_view_vector), 0.0f), u_materialShininess);"     \
        "       phong_ads_light = ambient + diffuse + specular;"                                                                        \
        "   }"                                                                                                                          \
        "   else"                                                                                                                       \
        "   {"                                                                                                                          \
        "       phong_ads_light = vec3(1.0f, 1.0f, 1.0f);"                                                                              \
        "   }"                                                                                                                          \
        "   fragColor = vec4(phong_ads_light, 1.0f);"                                                                                   \
        "}";

    //provide source code to shader object 
    glShaderSource(f_fragmentShaderObject, 1, (const GLchar**)&f_fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(f_fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(f_fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(f_fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(f_fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Fragment Shader Compiled Successfully -----\n");

    //--- Shader Program ---

    //create shader program
    f_shaderProgramObject = glCreateProgram();

    //attach vertex shader to shader program
    glAttachShader(f_shaderProgramObject, f_vertexShaderObject);

    //attach fragment shader to shader program
    glAttachShader(f_shaderProgramObject, f_fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(f_shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(f_shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

    //link shader program 
    glLinkProgram(f_shaderProgramObject);

    //shader linking error checking
    glGetProgramiv(f_shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(f_shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(f_shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    //get uniform locations
    f_modelMatrixUniform = glGetUniformLocation(f_shaderProgramObject, "u_modelMatrix");
    f_viewMatrixUniform = glGetUniformLocation(f_shaderProgramObject, "u_viewMatrix");
    f_perspectiveProjectionMatrixUniform = glGetUniformLocation(f_shaderProgramObject, "u_perspectiveProjectionMatrix");

    f_LaUniform = glGetUniformLocation(f_shaderProgramObject, "u_La");
    f_LdUniform = glGetUniformLocation(f_shaderProgramObject, "u_Ld");
    f_LsUniform = glGetUniformLocation(f_shaderProgramObject, "u_Ls");
    f_lightPositionUniform = glGetUniformLocation(f_shaderProgramObject, "u_lightPosition");

    f_KaUniform = glGetUniformLocation(f_shaderProgramObject, "u_Ka");
    f_KdUniform = glGetUniformLocation(f_shaderProgramObject, "u_Kd");
    f_KsUniform = glGetUniformLocation(f_shaderProgramObject, "u_Ks");
    f_materialShininessUniform = glGetUniformLocation(f_shaderProgramObject, "u_materialShininess");

    f_LKeyPressedUniform = glGetUniformLocation(f_shaderProgramObject, "u_LKeyPressed");

    //get buffer data for sphere
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_texcoords, sphere_elements);

    gNumVertices = getNumberOfSphereVertices();
    gNumElements = getNumberOfSphereElements();

    //setup vao and vbo for square
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);

    glGenBuffers(1, &vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_texcoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_texcoords), sphere_texcoords, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);

    //unbind buffers
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

    //initialize light properties
    La = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Ld = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Ls = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightPosition = vec4(100.0f, 100.0f, 100.0f, 1.0f);

    //initialize material properties
    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Ks = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    materialShininess = 50.0f;

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
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
    mat4 modelMatrix;
    mat4 viewMatrix;

    //code
    //clear the color buffer and depth buffer with currrent 
    //clearing values (set up in initilaize)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    //translate model matrix
    modelMatrix = vmath::translate(0.0f, 0.0f, -3.0f);

    if(perVertex_perFragmentToggle == false)
    {
        glUseProgram(v_shaderProgramObject);

        //pass the uniform variables to shaders 
        if(LKeyPressed == 1)
        {
            glUniform1i(v_LKeyPressedUniform, 1);

            glUniform3fv(v_LaUniform, 1, La);
            glUniform3fv(v_LdUniform, 1, Ld);
            glUniform3fv(v_LsUniform, 1, Ls);
            glUniform4fv(v_lightPositionUniform, 1, lightPosition);

            glUniform3fv(v_KaUniform, 1, Ka);
            glUniform3fv(v_KdUniform, 1, Kd);
            glUniform3fv(v_KsUniform, 1, Ks);
            glUniform1f(v_materialShininessUniform, materialShininess);
        }
        else
        {
            glUniform1i(v_LKeyPressedUniform, 0);
        }

        //pass model, view and projection matrices to shader uniform variables
        glUniformMatrix4fv(v_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(v_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(v_perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    }
    else
    {
        glUseProgram(f_shaderProgramObject);

        //pass the uniform variables to shaders 
        if(LKeyPressed == 1)
        {
            glUniform1i(f_LKeyPressedUniform, 1);

            glUniform3fv(f_LaUniform, 1, La);
            glUniform3fv(f_LdUniform, 1, Ld);
            glUniform3fv(f_LsUniform, 1, Ls);
            glUniform4fv(f_lightPositionUniform, 1, lightPosition);

            glUniform3fv(f_KaUniform, 1, Ka);
            glUniform3fv(f_KdUniform, 1, Kd);
            glUniform3fv(f_KsUniform, 1, Ks);
            glUniform1f(f_materialShininessUniform, materialShininess);
        }
        else
        {
            glUniform1i(f_LKeyPressedUniform, 0);
        }

        //pass model, view and projection matrices to shader uniform variables
        glUniformMatrix4fv(f_modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(f_viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(f_perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    }

    //bind vao
    glBindVertexArray(vao_sphere);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    //unbind 
    glBindVertexArray(0);
    glUseProgram(0);

    //swap the buffers
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

    //release vao and vbo for square
    if(vao_sphere)
    {
        glDeleteVertexArrays(1, &vao_sphere);
        vao_sphere = 0;
    }

    if(vbo_sphere_position)
    {
        glDeleteBuffers(1, &vbo_sphere_position);
        vbo_sphere_position = 0;
    }

    if(vbo_sphere_normal)
    {
        glDeleteBuffers(1, &vbo_sphere_normal);
        vbo_sphere_normal = 0;
    }

    if(vbo_sphere_texcoord)
    {
        glDeleteBuffers(1, &vbo_sphere_texcoord);
        vbo_sphere_texcoord = 0;
    }

    //safe shader cleanup of per vertex lighting 
    if(v_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(v_shaderProgramObject);
        glGetProgramiv(v_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(v_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(v_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(v_shaderProgramObject);
        v_shaderProgramObject = 0;
        glUseProgram(0);
    }

    //safe shader cleanup of per vertex lighting 
    if(f_shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(f_shaderProgramObject);
        glGetProgramiv(f_shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(f_shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(f_shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(f_shaderProgramObject);
        f_shaderProgramObject = 0;
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
