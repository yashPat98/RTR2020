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

GLuint vao_sphere;                 //handle to vertex array object for sphere
GLuint vbo_sphere_position;        //handle to vertex buffer object for vertices of sphere
GLuint vbo_sphere_normal;          //handle to vertex buffer object for normals of sphere
GLuint vbo_sphere_texcoord;        //handle to vertex buffer object for texcoords of sphere
GLuint vbo_sphere_elements;        //handle to vertex buffer object for indices 

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint perspectiveProjectionMatrixUniform;
GLuint LaUniform;
GLuint LdUniform;
GLuint LsUniform;
GLuint lightPositionUniform;
GLuint KaUniform;
GLuint KdUniform;
GLuint KsUniform;
GLuint materialShininessUniform;
GLuint LKeyPressedUniform;
               
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

GLfloat sphere_vertices[1146];
GLfloat sphere_normals[1146];
GLfloat sphere_texcoords[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

GLfloat angle_for_x_rotation;
GLfloat angle_for_y_rotation;
GLfloat angle_for_z_rotation;

int key_pressed;
int width;
int height;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                      //initialize OpenGL state machine
    void Display(void);                                         //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                        //structure holding window class attributes
    MSG msg;                                                    //structure holding message attributes
    HWND hwnd;                                                  //handle to a window
    TCHAR szAppName[] = TEXT("OpenGL : 24 Spheres");            //name of window class

    int cxScreen, cyScreen;                                     //screen width and height for centering window
    int init_x, init_y;                                         //top-left coordinates of centered window
    bool bDone = false;                                         //flag indicating whether or not to exit from game loop

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
            width = LOWORD(lParam);
            height = HIWORD(lParam);
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
                
                case 'x':
                case 'X':
                    key_pressed = 1;
                    angle_for_x_rotation = 0.0f;
                    break;
                
                case 'y':
                case 'Y':
                    key_pressed = 2;
                    angle_for_y_rotation = 0.0f;
                    break;

                case 'z':
                case 'Z':
                    key_pressed = 3;
                    angle_for_z_rotation = 0.0f;
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

    //--- Vertex Shader ---

    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                                                     \
        "\n"                                                                                                    \
        "in vec4 vPosition;"                                                                                    \
        "in vec3 vNormal;"                                                                                      \
        "uniform mat4 u_modelMatrix;"                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \
        "uniform vec4 u_lightPosition;"                                                                         \
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

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");

    //binding of shader program object with vertex shader normal attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

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
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    perspectiveProjectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_perspectiveProjectionMatrix");

    LaUniform = glGetUniformLocation(shaderProgramObject, "u_La");
    LdUniform = glGetUniformLocation(shaderProgramObject, "u_Ld");
    LsUniform = glGetUniformLocation(shaderProgramObject, "u_Ls");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPosition");

    KaUniform = glGetUniformLocation(shaderProgramObject, "u_Ka");
    KdUniform = glGetUniformLocation(shaderProgramObject, "u_Kd");
    KsUniform = glGetUniformLocation(shaderProgramObject, "u_Ks");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_materialShininess");

    LKeyPressedUniform = glGetUniformLocation(shaderProgramObject, "u_LKeyPressed");

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
    lightPosition = vec4(0.0f, 3.0f, 3.0f, 0.0f);

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
    //function declarations
    void DrawSpheres(void);

    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;

    GLfloat xCenter;
    GLfloat yCenter;

    //code
    //clear the color buffer and depth buffer with currrent 
    //clearing values (set up in initilaize)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);

    //set matrices to identity
    modelMatrix = mat4::identity();
    viewMatrix = mat4::identity();

    switch(key_pressed)
    {
        case 1:
            lightPosition[0] = 20.0f * sin(angle_for_x_rotation);
            lightPosition[1] = 20.0f * cos(angle_for_x_rotation);
            lightPosition[2] = 0.0f;
            break;
        
        case 2:
            lightPosition[0] = 20.0f * sin(angle_for_y_rotation);
            lightPosition[1] = 0.0f;
            lightPosition[2] = 20.0f * cos(angle_for_y_rotation);
            break;
        
        case 3:
            lightPosition[0] = 0.0f;
            lightPosition[1] = 20.0f * sin(angle_for_z_rotation);
            lightPosition[2] = 20.0f * cos(angle_for_z_rotation);
            break;
        
        default:
            break;
    }

    //pass the uniform variables to vertex shader
    if(LKeyPressed == 1)
    {
        glUniform1i(LKeyPressedUniform, 1);

        glUniform3fv(LaUniform, 1, La);
        glUniform3fv(LdUniform, 1, Ld);
        glUniform3fv(LsUniform, 1, Ls);
        glUniform4fv(lightPositionUniform, 1, lightPosition);
    }
    else
    {
        glUniform1i(LKeyPressedUniform, 0);
    }

    if(width <= height)
    {
        xCenter = 15.5f / 2.0f;
        yCenter = (15.5f * (GLfloat)height * 4.0f / (GLfloat)(width * 6.0f)) / 2.0f;
    }
    else
    {
        xCenter = (15.5f * (GLfloat)width * 6.0f / (GLfloat)(height * 4.0f)) / 2.0f;
        yCenter = 15.5f / 2.0f;
    }

    //translate model matrix
    modelMatrix = vmath::translate(0.0f, 0.0f, -2.0f);

    //pass model, view and projection matrices to shader uniform variables
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

    DrawSpheres();

    //unbind 
    glUseProgram(0);

    //update
    if(key_pressed == 1)
    {
        angle_for_x_rotation += 0.001f;
    }
    else if(key_pressed == 2)
    {
        angle_for_y_rotation += 0.001f;
    }
    else if(key_pressed == 3)
    {
        angle_for_z_rotation += 0.001f;
    }

    //swap the buffers
    SwapBuffers(ghdc);
}

void DrawSpheres(void)
{
    //variable declarations
    mat4 modelMatrix;

    //code
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Resize(width / 4, height / 6);

    //bind vao
    glBindVertexArray(vao_sphere);

    // --- Emerald ---
    glViewport(0, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.0215f, 0.1745f, 0.0215f, 1.0f);
    Kd = vec4(0.07568f, 0.61424f, 0.07568f, 1.0f);
    Ks = vec4(0.633, 0.727811f, 0.633f, 1.0f);
    materialShininess   = 0.6f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Jade ---
    glViewport(width / 4, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.135f, 0.2225f, 0.1575f, 1.0f);
    Kd = vec4(0.54f, 0.89f, 0.63f, 1.0f);
    Ks = vec4(0.316228f, 0.316228f, 0.316228f, 1.0f);
    materialShininess = 0.1f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Obsidian ---
    glViewport(width * 2 / 4, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.05375f, 0.05f, 0.06625f, 1.0f);
    Kd = vec4(0.18275f, 0.17f, 0.22525f, 1.0f);
    Ks = vec4(0.332741f, 0.328634f, 0.346435f, 1.0f);
    materialShininess = 0.3f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Pearl ---
    glViewport(width * 3 / 4, height * 5 / 6, width / 4, height / 6);

    Ka = vec4(0.25f, 0.20725f, 0.20725f, 1.0f);
    Kd = vec4(1.0f, 0.829f, 0.829f, 1.0f);
    Ks = vec4(0.296648f, 0.296648f, 0.296648f, 1.0f);
    materialShininess = 0.088f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Ruby ---
    glViewport(0, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.1745f, 0.01175f, 0.01175f, 1.0f);
    Kd = vec4(0.61424f, 0.04136f, 0.04136f, 1.0f);
    Ks = vec4(0.727811f, 0.626959f, 0.626959f, 1.0f);
    materialShininess   = 0.6f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Turquoise ---
    glViewport(width / 4, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.1f, 0.18725f, 0.1745f, 1.0f);
    Kd = vec4(0.396f, 0.74151f, 0.69102f, 1.0f);
    Ks = vec4(0.297254f, 0.30829f, 0.306678f, 1.0f);
    materialShininess = 0.1f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Brass ---
    glViewport(width * 2 / 4, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.329412f, 0.223529f, 0.027451f, 1.0f);
    Kd = vec4(0.780392f, 0.568627f, 0.113725f, 1.0f);
    Ks = vec4(0.992157f, 0.941176f, 0.807843f, 1.0f);
    materialShininess = 0.21794872f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Bronze ---
    glViewport(width * 3 / 4, height * 4 / 6, width / 4, height / 6);

    Ka = vec4(0.2125f, 0.1275f, 0.054f, 1.0f);
    Kd = vec4(0.714f, 0.4284f, 0.18144f, 1.0f);
    Ks = vec4(0.393548f, 0.271906f, 0.166721f, 1.0f);
    materialShininess  = 0.2f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Chrome ---
    glViewport(0, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.25f, 0.25f, 0.25f, 1.0f);
    Kd = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    Ks = vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
    materialShininess = 0.6f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Copper ---
    glViewport(width / 4, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.19125f, 0.0735f, 0.0225f, 1.0f);
    Kd = vec4(0.7038f, 0.27048f, 0.0828f, 1.0f);
    Ks = vec4(0.256777f, 0.137622f, 0.086014f, 1.0f);
    materialShininess = 0.1f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Gold ---
    glViewport(width * 2 / 4, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    Kd = vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
    Ks = vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
    materialShininess = 0.4f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Silver ---
    glViewport(width * 3 / 4, height * 3 / 6, width / 4, height / 6);

    Ka = vec4(0.19225f, 0.19225f, 0.19225f, 1.0f);
    Ks = vec4(0.50754f, 0.50754f, 0.50754f, 1.0f);
    Kd = vec4(0.508273f, 0.508273f, 0.508273f, 1.0f);
    materialShininess  = 0.4f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Black ---
    glViewport(0, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.01f, 0.01f, 0.01f, 1.0f);
    Ks = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Cyan ---
    glViewport(width / 4, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.1f, 0.06f, 1.0f);
    Kd = vec4(0.0f, 0.50980392f, 0.50980392f, 1.0f);
    Ks = vec4(0.50196078f, 0.50196078f, 0.50196078f, 1.0f);
    materialShininess = 0.25f * 128.0f;    

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Green ---
    glViewport(width * 2 / 4, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.1f, 0.35f, 0.1f, 1.0f);
    Ks = vec4(0.45f, 0.55f, 0.45f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Red ---
    glViewport(width * 3 / 4, height * 2 / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.0f, 0.0f, 1.0f);
    Ks = vec4(0.7f, 0.6f, 0.6f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- White ---
    glViewport(0, height / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.55f, 0.55f, 0.55f, 1.0f);
    Ks = vec4(0.7f, 0.7f, 0.7f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Yellow Plastic ---
    glViewport(width / 4, height / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.5f, 0.0f, 1.0f);
    Ks = vec4(0.6f, 0.6f, 0.5f, 1.0f);
    materialShininess = 0.25f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Black ---
    glViewport(width * 2 / 4, height / 6, width / 4, height / 6);

    Ka = vec4(0.02f, 0.02f, 0.02f, 1.0f);
    Kd = vec4(0.01f, 0.01f, 0.01f, 1.0f);
    Ks = vec4(0.4f, 0.4f, 0.4f, 1.0f);
    materialShininess = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Cyan ---
    glViewport(width * 3 / 4, height / 6, width / 4, height / 6);

    Ka = vec4(0.0f, 0.05f, 0.05f, 1.0f);
    Kd = vec4(0.4f, 0.5f, 0.5f, 1.0f);
    Ks = vec4(0.04f, 0.7f, 0.7f, 1.0f);
    materialShininess   = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Green ---
    glViewport(0, 0, width / 4, height / 6);

    Ka = vec4(0.0f, 0.05f, 0.0f, 1.0f);
    Kd = vec4(0.4f, 0.5f, 0.4f, 1.0f);
    Ks = vec4(0.04f, 0.7f, 0.04f, 1.0f);
    materialShininess   = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Red ---
    glViewport(width / 4, 0, width / 4, height / 6);

    Ka = vec4(0.05f, 0.0f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.4f, 0.4f, 1.0f);
    Ks = vec4(0.7f, 0.04f, 0.04f, 1.0f);
    materialShininess   = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- White ---
    glViewport(width * 2 / 4, 0, width / 4, height / 6);

    Ka = vec4(0.05f, 0.05f, 0.05f, 1.0f);
    Kd = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    Ks = vec4(0.7f, 0.7f, 0.7f, 1.0f);
    materialShininess = 0.078125f * 128.0f;

    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    // --- Yellow Rubber ---
    glViewport(width * 3 / 4, 0, width / 4, height / 6);

    Ka = vec4(0.05f, 0.05f, 0.0f, 1.0f);
    Kd = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    Ks = vec4(0.7f, 0.7f, 0.04f, 1.0f);
    materialShininess = 0.078125f * 128.0f;
    
    glUniform3fv(KaUniform, 1, Ka);
    glUniform3fv(KdUniform, 1, Kd);
    glUniform3fv(KsUniform, 1, Ks);
    glUniform1f(materialShininessUniform, materialShininess);

    //draw sphere
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_elements);
    glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, (void*)0);

    glBindVertexArray(0);
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
