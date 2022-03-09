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

#define BUFFER_SIZE 1024

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

struct vec_int
{
    int *data;
    int size;
};

struct vec_float
{
    float *data;
    int size;
};

struct ModelData
{
    struct vec_int *vec_int_vertices = NULL;
    struct vec_int *vec_int_normals = NULL;
    struct vec_int *vec_int_texcoords = NULL;
    struct vec_float *vec_float_vertices = NULL;
    struct vec_float *vec_float_normals = NULL;
    struct vec_float *vec_float_texcoords = NULL;
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
GLuint textureSamplerUniform;

GLuint vao_model;
GLuint vbo_position_model;
GLuint vbo_texture_model;
GLuint vbo_normal_model;
GLuint vbo_elements_model;

mat4 perspectiveProjectionMatrix;  
GLuint monkeyhead_texture;

char model_buffer[BUFFER_SIZE];
ModelData monkeyhead;

int LKeyPressed;

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
    TCHAR szAppName[] = TEXT("Perspective");               //name of window class

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
                case 'l':
                case 'L':
                    LKeyPressed = !LKeyPressed;
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
    struct ModelData load_static_model(const char *modelFilePathWithName);

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
        "in vec2 vTexcoords;"                                                                                   \

        "uniform mat4 u_modelMatrix;"                                                                           \
        "uniform mat4 u_viewMatrix;"                                                                            \
        "uniform mat4 u_perspectiveProjectionMatrix;"                                                           \
        
        "uniform vec4 u_lightPosition;"
        "uniform int u_LKeyPressed;"                                                                            \
        
        "out vec3 transformed_normal;"                                                                          \
        "out vec3 light_direction;"                                                                             \
        "out vec3 view_vector;"                                                                                 \
        "out vec2 texcoords;"                                                                                   \

        "void main(void)"                                                                                       \
        "{"                                                                                                     \
        "   if(u_LKeyPressed == 1)"                                                                             \
        "   {"                                                                                                  \
        "       vec4 eye_coords = u_viewMatrix * u_modelMatrix * vPosition;"                                    \
        "       mat3 normal_matrix = mat3(transpose(inverse(u_viewMatrix * u_modelMatrix))); "                  \
        "       transformed_normal = normal_matrix * vNormal;"                                                  \
        "       light_direction = vec3(u_lightPosition - eye_coords);"                                          \
        "       view_vector = -eye_coords.xyz;"                                                                 \
        "       texcoords = vTexcoords;"                                                                        \
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
        "in vec2 texcoords;"                                                                                                            \

        "uniform vec3 u_La;"                                                                                                            \
        "uniform vec3 u_Ld;"                                                                                                            \
        "uniform vec3 u_Ls;"                                                                                                            \
        "uniform vec3 u_Ka;"                                                                                                            \
        "uniform vec3 u_Kd;"                                                                                                            \
        "uniform vec3 u_Ks;"                                                                                                            \
        "uniform float u_materialShininess;"                                                                                            \
        "uniform int u_LKeyPressed;"                                                                                                    \
        
        "uniform sampler2D textureSampler;"                                                                                             \

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
        "       vec3 texcolor = texture(textureSampler,texcoords).rgb;"                                                                 \
        "       vec3 ambient = u_La * u_Ka * texcolor;"                                                                                 \
        "       vec3 diffuse = u_Ld * u_Kd  * max(dot(normalized_light_direction, normalized_transformed_normal), 0.0f) * texcolor;"    \
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
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexcoords");

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
    textureSamplerUniform = glGetUniformLocation(shaderProgramObject, "textureSampler");

    //geometry data
    monkeyhead = load_static_model("MonkeyHead.obj");

    //setup vao and vbo
	glGenVertexArrays(1, &vao_model);
	glBindVertexArray(vao_model);

	glGenBuffers(1, &vbo_position_model);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_model);
	glBufferData(GL_ARRAY_BUFFER, monkeyhead.vec_float_vertices->size * sizeof(float), monkeyhead.vec_float_vertices->data, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_normal_model);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_model);
	glBufferData(GL_ARRAY_BUFFER, monkeyhead.vec_float_normals->size * sizeof(float), monkeyhead.vec_float_normals->data, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_texture_model);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_model);
	glBufferData(GL_ARRAY_BUFFER, monkeyhead.vec_float_texcoords->size * sizeof(float), monkeyhead.vec_float_texcoords->data, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_elements_model);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements_model);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, monkeyhead.vec_int_vertices->size * sizeof(int), monkeyhead.vec_int_vertices->data, GL_STATIC_DRAW);
 
	glBindVertexArray(0);

    //smooth shading  
    glShadeModel(GL_SMOOTH);                  

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    
    glEnable(GL_CULL_FACE);

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();

    loadGLTexture(&monkeyhead_texture, MAKEINTRESOURCE(MONKEYHEAD_BITMAP));

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

struct ModelData load_static_model(const char *modelFilePathWithName)
{
    // Local Function Declaration
    struct vec_int *create_vec_int(void);
    void push_vec_int(struct vec_int *, int);
    void destroy_vec_int(struct vec_int *);

    struct vec_float *create_vec_float(void);
    void push_vec_float(struct vec_float *, float);
    void destroy_vec_float(struct vec_float *);

    // Local Variable Declaration
    FILE *modelFile = NULL;
    struct ModelData modelData;

    int index;
    char *first_token = NULL;
    char *token = NULL;
    char *space = " ";
    char *slash = "/";
    char *face_values[] = {NULL, NULL, NULL};

    // Code
    modelFile = fopen(modelFilePathWithName, "r+");
    if (modelFile == NULL)
    {
        fprintf(gpFile, "[%s - LINE - %d] error>> fopen(%s) exiting...\n", __FILE__, __LINE__, modelFilePathWithName);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "info>> fopen(%s) sucessful!\n", modelFilePathWithName);
    }

    modelData.vec_float_vertices = create_vec_float();
    modelData.vec_float_normals = create_vec_float();
    modelData.vec_float_texcoords = create_vec_float();

    modelData.vec_int_vertices = create_vec_int();
    modelData.vec_int_normals = create_vec_int();
    modelData.vec_int_texcoords = create_vec_int();

    while (fgets(model_buffer, BUFFER_SIZE, modelFile) != NULL)
    {
        first_token = strtok(model_buffer, space);

        if (strcmp(first_token, "v") == 0)
        {
            while ((token = strtok(NULL, space)) != NULL)
            {
                push_vec_float(modelData.vec_float_vertices, atof(token));
            }
        }

        else if (strcmp(first_token, "vn") == 0)
        {
            while ((token = strtok(NULL, space)) != NULL)
            {
                push_vec_float(modelData.vec_float_normals, atof(token));
            }
        }

        else if (strcmp(first_token, "vt") == 0)
        {
            while ((token = strtok(NULL, space)) != NULL)
            {
                push_vec_float(modelData.vec_float_texcoords, atof(token));
            }
        }

        else if (strcmp(first_token, "f") == 0)
        {
            for (index = 0; index < 3; index++)
            {
                face_values[index] = strtok(NULL, space);
            }
            for (int index = 0; index < 3; index++)
            {
                token = strtok(face_values[index], slash);
                push_vec_int(modelData.vec_int_vertices, atoi(token) - 1);

                token = strtok(NULL, slash);
                push_vec_int(modelData.vec_int_normals, atoi(token) - 1);

                token = strtok(NULL, slash);
                push_vec_int(modelData.vec_int_texcoords, atoi(token) - 1);
            }
        }

        memset(model_buffer, 0, sizeof(BUFFER_SIZE));
    }

    fclose(modelFile);

    return (modelData);
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
    mat4 modelMatrix;
    mat4 viewMatrix;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);

        //set matrices to identity
        modelMatrix = mat4::identity();
        viewMatrix = mat4::identity();

        modelMatrix = vmath::translate(0.0f, 0.0f, -5.0f);
 
        //pass model, view and projection matrices to shader uniform variables
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(perspectiveProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        if(LKeyPressed == 1)
        {
            glUniform1i(LKeyPressedUniform, 1);

            glUniform3f(LaUniform, 0.0f, 0.0f, 0.0f);
            glUniform3f(LdUniform, 1.0f, 1.0f, 1.0f);
            glUniform3f(LsUniform, 1.0f, 1.0f, 1.0f);
            glUniform4f(lightPositionUniform, 100.0f, 100.0f, 100.0f, 1.0f);

            glUniform3f(KaUniform, 0.0f, 0.0f, 0.0f);
            glUniform3f(KdUniform, 1.0f, 1.0f, 1.0f);
            glUniform3f(KsUniform, 1.0f, 1.0f, 1.0f);
            glUniform1f(materialShininessUniform, 128.0f);
        }
        else
        {
            glUniform1i(LKeyPressedUniform, 0);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, monkeyhead_texture);
        glUniform1i(textureSamplerUniform, 0);

        glBindVertexArray(vao_model);
        //glDrawArrays(GL_TRIANGLES, 0, monkeyhead.vec_float_vertices->size);
        glDrawElements(GL_TRIANGLES, monkeyhead.vec_int_vertices->size, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    glUseProgram(0);

    SwapBuffers(ghdc);
}

void UnInitialize(void)
{
    //function declarations
    void destroy_model_arrays(struct ModelData *modelData);

    //code
    destroy_model_arrays(&monkeyhead);

    if(monkeyhead_texture)
    {
        glDeleteTextures(1, &monkeyhead_texture);
        monkeyhead_texture = 0;
    }

    if (vao_model)
	{
		glDeleteVertexArrays(1, &vao_model);
		vao_model = 0;
	}

	if (vbo_position_model)
	{
		glDeleteBuffers(1, &vbo_position_model);
		vbo_position_model = 0;
	}

	if (vbo_texture_model)
	{
		glDeleteVertexArrays(1, &vbo_texture_model);
		vbo_texture_model = 0;
	}

	if (vbo_normal_model)
	{
		glDeleteBuffers(1, &vbo_normal_model);
		vbo_normal_model = 0;
	}

	if (vbo_elements_model)
	{
		glDeleteBuffers(1, &vbo_elements_model);
		vbo_elements_model = 0;
	}

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

struct vec_int *create_vec_int(void)
{
    // Code
    struct vec_int *ptr_struct_vec_int = NULL;

    ptr_struct_vec_int = (struct vec_int *)malloc(sizeof(struct vec_int));
    memset(ptr_struct_vec_int, 0, sizeof(struct vec_int));

    return (ptr_struct_vec_int);
}

void push_vec_int(struct vec_int *ptr_struct_vec_int, int data)
{
    // Code
    ptr_struct_vec_int->data = (int *)realloc((ptr_struct_vec_int->data), ((ptr_struct_vec_int->size) + 1) * sizeof(int));
    ptr_struct_vec_int->size = ptr_struct_vec_int->size + 1;
    ptr_struct_vec_int->data[(ptr_struct_vec_int->size) - 1] = data;
}

void destroy_vec_int(struct vec_int *ptr_struct_vec_int)
{
    // Code
    free(ptr_struct_vec_int->data);
    free(ptr_struct_vec_int);

    ptr_struct_vec_int = NULL;
}

struct vec_float *create_vec_float(void)
{
    // Code
    struct vec_float *ptr_struct_vec_float = NULL;
    ptr_struct_vec_float = (struct vec_float *)malloc(sizeof(struct vec_float));
    memset(ptr_struct_vec_float, 0, sizeof(struct vec_float));

    return (ptr_struct_vec_float);
}

void push_vec_float(struct vec_float *ptr_struct_vec_float, float data)
{
    // Code
    ptr_struct_vec_float->data = (float *)realloc(ptr_struct_vec_float->data, ((ptr_struct_vec_float->size) + 1) * sizeof(float));
    ptr_struct_vec_float->size = ptr_struct_vec_float->size + 1;
    ptr_struct_vec_float->data[(ptr_struct_vec_float->size) - 1] = data;
}

void destroy_vec_float(struct vec_float *ptr_struct_vec_float)
{
    // Code
    free(ptr_struct_vec_float->data);
    free(ptr_struct_vec_float);

    ptr_struct_vec_float = NULL;
}

void destroy_model_arrays(struct ModelData *modelData)
{
    // Code
    if (modelData->vec_float_vertices)
    {
        destroy_vec_float(modelData->vec_float_vertices);
        modelData->vec_float_vertices = NULL;
    }

    if (modelData->vec_float_normals)
    {
        destroy_vec_float(modelData->vec_float_normals);
        modelData->vec_float_normals = NULL;
    }

    if (modelData->vec_float_texcoords)
    {
        destroy_vec_float(modelData->vec_float_texcoords);
        modelData->vec_float_texcoords = NULL;
    }

    if (modelData->vec_int_vertices)
    {
        destroy_vec_int(modelData->vec_int_vertices);
        modelData->vec_int_vertices = NULL;
    }

    if (modelData->vec_int_normals)
    {
        destroy_vec_int(modelData->vec_int_normals);
        modelData->vec_int_normals = NULL;
    }

    if (modelData->vec_int_texcoords)
    {
        destroy_vec_int(modelData->vec_int_texcoords);
        modelData->vec_int_normals = NULL;
    }
}

