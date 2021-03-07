//headers
#include <windows.h>               //standard windows header
#include <gl/GL.h>                 //OpenGL header
#include <gl/GLU.h>                //OpenGL utility header
#include <stdio.h>                 //C header 
#include <stdlib.h>                //C header
#include "MeshLoading.h"           //Resources header

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

//symbolic constants
#define WIN_WIDTH  800                    //initial width of window  
#define WIN_HEIGHT 600                    //initial height of window

#define VK_F       0x46                   //virtual key code of F key
#define VK_f       0x60                   //virtual key code of f key

#define BUFFER_SIZE 256                   //maximum length of a line in a mesh file
#define S_EQUAL       0                   //return value of strcmp if equal

#define MONKEYHEAD_X_TRANSLATE     0.0f   //X-translation of monkeyhead
#define MONKEYHEAD_Y_TRANSLATE    -0.0f   //Y-translation of monkeyhead
#define MONKEYHEAD_Z_TRANSLATE    -5.0f   //Z-translation of monkeyhead

#define MONKEYHEAD_X_SCALE_FACTOR  1.5f   //X-scale factor of monkeyhead
#define MONKEYHEAD_Y_SCALE_FACTOR  1.5f   //Y-scale factor of monkeyhead
#define MONKEYHEAD_Z_SCALE_FACTOR  1.5f   //Z-scale factor of monkeyhead

#define NR_POINT_COORDS              3    //number of point coordinates
#define NR_TEXTURE_COORDS            2    //number of texture coordinates
#define NR_NORMAL_COORDS             3    //number of normal coordinates
#define NR_FACE_TOKENS               3    //number of entries in face data
#define NR_TRIANGLE_VERTICES         3    //number of vertices in a triangle

//callback procedure declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//structure definitions
typedef struct Vector2D_integer           //vector of vector of integers
{
    GLint **pp_arr;
    size_t size;
}Vector2Di;

typedef struct Vector2D_float             //vector of vector of floats
{
    GLfloat **pp_arr;
    size_t size;
}Vector2Df;

//global variables
HWND   ghwnd  = NULL;                     //handle to a window
HDC    ghdc   = NULL;                     //handle to a device context
HGLRC  ghrc   = NULL;                     //handle to a rendering context

DWORD dwStyle = NULL;                     //window style
WINDOWPLACEMENT wpPrev;                   //structure for holding previous window position

bool gbActiveWindow = false;              //flag indicating whether window is active or not
bool gbFullscreen = false;                //flag indicating whether window is fullscreen or not

FILE*  gpFile = NULL;                     //handle to a log file

FILE* gpMeshfile = NULL;                  //handle to a mesh file

Vector2Df *gpVertices       = NULL;       //vertex data of mesh
Vector2Df *gpTexCoords      = NULL;       //tex-coord data of mesh
Vector2Df *gpNormals        = NULL;       //normal data of mesh

Vector2Di *gpFaceTriangles  = NULL;       //indices of vertices of faces
Vector2Di *gpFaceTexCoords  = NULL;       //indices of tex-coords of faces
Vector2Di *gpFaceNormals    = NULL;       //indices of normals of faces

char buffer[BUFFER_SIZE];                 //buffer for parsing file

bool gbAnimate  = false;                  //toggle animation
bool gbLight    = false;                  //toggle lighting
bool gbTexture  = false;                  //toggle texturing

GLfloat lightAmbient[]  = {0.0f, 0.0f, 0.0f, 1.0f};      //ambient RGBA intensity of light 0
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};      //diffuse RGBA intensity of light 0
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};      //specular RGBA intensity of light 0
GLfloat lightPosition[] = {5.0f, 5.0f, 5.0f, 1.0f};      //poisition of light 0

GLuint monkeyhead_texture;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                     //initialize OpenGL state machine
    void Display(void);                        //render scene

    //variable declarations
    WNDCLASSEX wndclass;                       //structure holding window class attributes
    MSG msg;                                   //structure holding message attributes
    HWND hwnd;                                 //handle to a window
    TCHAR szAppName[] = TEXT("Mesh Loading");  //name of window class
    int cxScreen, cyScreen;                    //screen width and height for centering window
    int init_x, init_y;                        //top-left coordinates of centered window
    bool bDone = false;                        //flag indicating whether or not to exit from game loop

    //code
    //create/open  'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Failed to open log.txt file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "Program started successfully.\n\n");
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

        case WM_CHAR:                            //event : a key is pressed but handle as character
            switch(wParam)
            {
                case 'A':
                case 'a':
                    //toggle rotation
                    if(gbAnimate == false)
                    {
                        gbAnimate = true;
                    }
                    else
                    {
                        gbAnimate = false;
                    }
                    break;

                case 'T':
                case 't':
                    //toggle texturing
                    if(gbTexture == false)
                    {
                        glEnable(GL_TEXTURE_2D);
                        gbTexture = true;
                    }
                    else
                    {
                        glDisable(GL_TEXTURE_2D);
                        gbTexture = false;
                    }
                    break;
                
                case 'L':
                case 'l':
                    //toggle lighting 
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

        case WM_KEYDOWN:                         //event : a key has been pressed but handle as virtual key code
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
    void Resize(int, int);                                          //warm-up call
    void UnInitialize(void);                                        //release resources
    bool loadGLTexture(GLuint *texture, TCHAR ResourceID[]);        //load texture from resource file (.rc file)
    void LoadMeshData(void);                                        //load mesh data from wavfront (.obj file)

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

    //setup render scene

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

    //smooth shading  
    glShadeModel(GL_SMOOTH);  

    //set up light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);

    //load texture for monkeyhead
    loadGLTexture(&monkeyhead_texture, MAKEINTRESOURCE(MONKEYHEAD_BITMAP));

    //load mesh
    LoadMeshData();

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

bool loadGLTexture(GLuint *texture, TCHAR ResourceID[])
{
    //variable declaration
    bool bResult = false;                   //status of texture loading
    HBITMAP hBitmap = NULL;                 //handle to bitmap 
    BITMAP bmp;                             //structure containg header of bitmap

    //code
    //load bmp image from resource file to handle 
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), ResourceID, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if(hBitmap)
    {
        //texture is successfully acquired
        bResult = true;

        //get the header of bitmap for texture information
        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        //pixel storage mode 
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        //generate texture name i.e unused unsigned
        //integers representing a texture object
        glGenTextures(1, texture);

        //bind the texture name to texture memory and 
        //subsequent calls to set texture parameters
        //will set the state of binded texture object parameters
        glBindTexture(GL_TEXTURE_2D, *texture);

        //setting texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        //push the data to texture memory
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

        //release handle to bitmap
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    return (bResult);
}

void LoadMeshData(void)
{
    //function declarations
    Vector2Di *createVector2Di(void);
    Vector2Df *createVector2Df(void);
    void pushbackVector2Di(Vector2Di *p_vec, GLint *p_arr);
    void pushbackVector2Df(Vector2Df *p_vec, GLfloat *p_arr);
    void cleanVector2Di(Vector2Di **pp_vec);
    void cleanVector2Df(Vector2Df **pp_vec);
    void *xcalloc(int nr_elements, size_t size_per_element);

    //variable declaraions
    char *sep_space = " ";                     //space separator for strtok 
    char *sep_fslash = "/";                    //forward slash seperator for  strtok
    char *first_token = NULL;                  //first word in a line (v, vt, vn, f)
    char *token = NULL;                        //next word in a line (data)
    char *face_tokens[NR_FACE_TOKENS];         //array of  character pointers to hold string of face entries
    char nr_tokens;                            //number of non-null tokens in the above vector
    char *token_vertex_index = NULL;           //string associated with vertex index
    char *token_texture_index = NULL;          //string associated with tex-coord index
    char *token_normal_index = NULL;           //string associated with normal index

    //code
    //open mesh file
    gpMeshfile = fopen("MonkeyHead.obj", "r");
    if(!gpMeshfile)
    {
        fprintf(gpFile, "Error : failed to open MonkeyHead.obj file.\n");
        DestroyWindow(ghwnd);
    }

    //allocate memory for vector of vector of floats to hold
    //vertex, tex-coord and normal data
    gpVertices = createVector2Df();
    gpTexCoords = createVector2Df();
    gpNormals = createVector2Df();

    //allocate memory for vector of vector of integers to hold
    //indices of face data (vertices, tex-coords and normals)
    gpFaceTriangles = createVector2Di();
    gpFaceTexCoords = createVector2Di();
    gpFaceNormals = createVector2Di();

    //parse through the file
    while(fgets(buffer, BUFFER_SIZE, gpMeshfile) != NULL)
    {
        //bind line to strtok with space seperator and get first token 
        first_token = strtok(buffer, sep_space);

        //if first token indicates vertex data (x, y, z)
        if(strcmp(first_token, "v") == S_EQUAL)
        {
            //create vector of NR_POINT_COORDS number of floats
            GLfloat *pvec_point_coord = (GLfloat *)xcalloc(NR_POINT_COORDS, sizeof(GLfloat));

            //get the next token from same line 
            //convert the char string to float 
            //and add that data to vector
            for(int i = 0; i != NR_POINT_COORDS; i++)
                pvec_point_coord[i] = atof(strtok(NULL, sep_space));

            //push the vector to global vector of vector of floats gpVertices
            pushbackVector2Df(gpVertices, pvec_point_coord);
        }

        //if the first token indicates texture coordinates (s, t)
        else if(strcmp(first_token, "vt") == S_EQUAL)
        {
            //create vector of NR_TEXTURE_COORDS of floats
            GLfloat *pvec_texture_coord = (GLfloat *)xcalloc(NR_TEXTURE_COORDS, sizeof(GLfloat));

            //get the next token from same line 
            //convert the char string to float 
            //and add that data to vector
            for(int i = 0; i != NR_TEXTURE_COORDS; i++)
                pvec_texture_coord[i] = atof(strtok(NULL, sep_space));
            
            //push the vector to global vector of vector of floats gpTexCoords
            pushbackVector2Df(gpTexCoords, pvec_texture_coord);
        }

        //if first token indicates normal data
        else if(strcmp(first_token, "vn") == S_EQUAL)
        {
            //create vector of NR_NORMAL_COORDS of floats
            GLfloat *pvec_normal_coord = (GLfloat *)xcalloc(NR_NORMAL_COORDS, sizeof(GLfloat));
        
            //get the next token from same line 
            //convert the char string to float 
            //and add that data to vector
            for(int i = 0; i != NR_NORMAL_COORDS; i++)
                pvec_normal_coord[i] = atof(strtok(NULL, sep_space));

            //push the vector to global vector of vector of floats gpNormals
            pushbackVector2Df(gpNormals, pvec_normal_coord);
        }

        //if first tokens indicates face data
        else if(strcmp(first_token, "f") == S_EQUAL)
        {
            //define three vectors of integers with length 3 to hold indices
            //of triangle's vertices, text-coords and normals
            GLint *pvec_vertex_indices    = (GLint *)xcalloc(3, sizeof(GLint));
            GLint *pvec_texcoord_indices  = (GLint *)xcalloc(3, sizeof(GLint));
            GLint *pvec_normal_indices    = (GLint *)xcalloc(3, sizeof(GLint));
        
            //clear face tokens
            memset((void *)face_tokens, 0, NR_FACE_TOKENS);

            //extraxt face information in face_tokens
            //and increment nr_tokens accordingly
            nr_tokens = 0;
            while(token = strtok(NULL, sep_space))
            {
                if(strlen(token) < 3)
                    break;
                
                face_tokens[nr_tokens] = token;
                nr_tokens++;
            }

            //extract single face token in loop
            //and get the indices of triangle data 
            //and store them in vectors (V/T/N)
            for(int i = 0; i != NR_FACE_TOKENS; i++)
            {
                token_vertex_index = strtok(face_tokens[i], sep_fslash);
                token_texture_index = strtok(NULL, sep_fslash);
                token_normal_index = strtok(NULL, sep_fslash);

                pvec_vertex_indices[i] = atoi(token_vertex_index);
                pvec_texcoord_indices[i] = atoi(token_texture_index);
                pvec_normal_indices[i] = atoi(token_normal_index);
            }

            pushbackVector2Di(gpFaceTriangles, pvec_vertex_indices);
            pushbackVector2Di(gpFaceTexCoords, pvec_texcoord_indices);
            pushbackVector2Di(gpFaceNormals, pvec_normal_indices);
        }

        //clear buffer 
        memset((void *)buffer, (int)'\0', BUFFER_SIZE);
    }

    //close the meshfile
    fclose(gpMeshfile);
    gpMeshfile = NULL;

    //log vertex, texture, normal and face data in a log file
    fprintf(gpFile, "Meshfile name : MonkeyHead.obj\n");
    fprintf(gpFile, "Vertices   : %zu\n", gpVertices->size);
    fprintf(gpFile, "Tex-Coords : %zu\n", gpTexCoords->size);
    fprintf(gpFile, "Normals    : %zu\n", gpNormals->size);
    fprintf(gpFile, "Triangles  : %zu\n", gpFaceTriangles->size);
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

    //load identity matrix on projection matrix stack
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //set perspective projection (internally calls glFrustum)
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void Display(void)
{
    //variable declarations
    static GLfloat rotation = 0.0f;

    //code
    //clear color and depth buffers asynchronously (implementation dependent)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //load identity matrix on modelview matrix stack
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //mesh transformations
    glTranslatef(MONKEYHEAD_X_TRANSLATE, MONKEYHEAD_Y_TRANSLATE, MONKEYHEAD_Z_TRANSLATE);
    glRotatef(rotation, 0.0f, 1.0f, 0.0f);
    glScalef(MONKEYHEAD_X_SCALE_FACTOR, MONKEYHEAD_Y_SCALE_FACTOR, MONKEYHEAD_Z_SCALE_FACTOR);

    //triangles in mesh has counter-clockwise windings
    glFrontFace(GL_CCW);

    //bind texture for monkeyhead model
    glBindTexture(GL_TEXTURE_2D, monkeyhead_texture);

    for(int i = 0; i != gpFaceTriangles->size; i++)
    {
        glBegin(GL_TRIANGLES);
            for(int j = 0; j != NR_TRIANGLE_VERTICES; j++)
            {
                //indices in mesh file are 1 based 
                //but array has 0 based indices 
                int vi = gpFaceTriangles->pp_arr[i][j] - 1;
                int ti = gpFaceTexCoords->pp_arr[i][j] - 1;
                int ni = gpFaceNormals->pp_arr[i][j] - 1;

                //using above indices of vertex, tex-coords and normals
                //push the data for rendering 
                glNormal3f((gpNormals->pp_arr[ni])[0], (gpNormals->pp_arr[ni])[1], (gpNormals->pp_arr[ni])[2]);
                glTexCoord2f((gpTexCoords->pp_arr[ti])[0], (gpTexCoords->pp_arr[ti])[1]);
                glVertex3f((gpVertices->pp_arr[vi])[0], (gpVertices->pp_arr[vi])[1], (gpVertices->pp_arr[vi])[2]);
            }
        glEnd();
    }

    //update
    if(gbAnimate)
    {
        //increment the rotation of y axis
        //clamp between 0 and 360 to avoid
        //out of range increment
        rotation += 1.0f;
        if(rotation >= 360.0f)
            rotation = 0.0f;
    }

    //swap front and back buffers
    SwapBuffers(ghdc);
}

#pragma region vector implementation

Vector2Di *createVector2Di(void)
{
    //function declarations
    void *xcalloc(int nr_elements, size_t size_per_element);

    //code
    return ((Vector2Di *)xcalloc(1, sizeof(Vector2Di)));
}

Vector2Df *createVector2Df(void)
{
    //function declarations
    void *xcalloc(int nr_elements, size_t size_per_element);

    //code
    return ((Vector2Df *)xcalloc(1, sizeof(Vector2Df)));
}

void pushbackVector2Di(Vector2Di *p_vec, GLint *p_arr)
{
    //function declarations
    void *xrealloc(void *p, size_t new_size);

    //code
    p_vec->pp_arr = (GLint **)xrealloc(p_vec->pp_arr, (p_vec->size + 1) * sizeof(GLint *));
    p_vec->pp_arr[p_vec->size] = p_arr;
    p_vec->size++;
}

void pushbackVector2Df(Vector2Df *p_vec, GLfloat *p_arr)
{
    //function declarations
    void *xrealloc(void *p, size_t new_size);

    //code
    p_vec->pp_arr = (GLfloat **)xrealloc(p_vec->pp_arr, (p_vec->size + 1) * sizeof(GLfloat *));
    p_vec->pp_arr[p_vec->size] = p_arr;
    p_vec->size++;
}

void cleanVector2Di(Vector2Di **pp_vec)
{
    //code
    Vector2Di *p_vec = *pp_vec;
    
    if(p_vec)
    {
        for(size_t i = 0; i != p_vec->size; i++)
        {
            if(p_vec->pp_arr[i])
                free(p_vec->pp_arr[i]);
        }
    
        free(p_vec);
        *pp_vec = NULL;
    }
}

void cleanVector2Df(Vector2Df **pp_vec)
{
    //code
    Vector2Df *p_vec = *pp_vec;

    if(p_vec)
    {
        for(size_t i = 0; i != p_vec->size; i++)
        {
            if(p_vec->pp_arr[i])
                free(p_vec->pp_arr[i]);
        }

        free(p_vec);
        *pp_vec = NULL;
    }    
}

void *xcalloc(int nr_elements, size_t size_per_element)
{
    //code
    void *p = calloc(nr_elements, size_per_element);
    if(!p)
    {
        fprintf(gpFile, "Fatal Error : calloc() failed to allocate memory.\n");
        DestroyWindow(ghwnd);
    }

    return (p);
}

void *xrealloc(void *p, size_t new_size)
{
    //code
    void *ptr = realloc(p, new_size);
    if(!ptr)
    {
        fprintf(gpFile, "Fatal Error : realloc() failed to allocate memory.\n");
        DestroyWindow(ghwnd);
    }

    return (ptr);
}

#pragma endregion

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

    //delete texture object
    glDeleteTextures(1, &monkeyhead_texture);

    //clean mesh data
    cleanVector2Df(&gpVertices);
    cleanVector2Df(&gpTexCoords);
    cleanVector2Df(&gpNormals);
    cleanVector2Di(&gpFaceTriangles);
    cleanVector2Di(&gpFaceTexCoords);
    cleanVector2Di(&gpFaceNormals);

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
        fprintf(gpFile, "\nProgram completed successfully.\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}


