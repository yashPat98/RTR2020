//headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <SOIL/SOIL.h>

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

//namespaces
using namespace std;

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

//global variable declarations
Display      *gpDisplay     = NULL;
XVisualInfo  *gpXVisualInfo = NULL;
Colormap      gColormap;
Window        gWindow;
GLXContext    gGLXContext;

int giWindowHeight = 600;
int giWindowWidth  = 800;
bool gbFullscreen  = false;
                   
FILE* gpMeshfile = NULL;                 

Vector2Df *gpVertices       = NULL;       
Vector2Df *gpTexCoords      = NULL;       
Vector2Df *gpNormals        = NULL;       

Vector2Di *gpFaceTriangles  = NULL;       
Vector2Di *gpFaceTexCoords  = NULL;       
Vector2Di *gpFaceNormals    = NULL;       

char buffer[BUFFER_SIZE];                 

GLfloat lightAmbient[]  = {0.0f, 0.0f, 0.0f, 1.0f};      
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};      
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};      
GLfloat lightPosition[] = {5.0f, 5.0f, 5.0f, 1.0f};      

GLuint monkeyhead_texture;

bool gbTexture          = false;                         
bool gbLight            = false;                         
bool gbAnimate          = false;

//entry-point function
int main(void)
{
    //function prototypes
    void CreateWindow(void);
    void ToggleFullscreen(void);
    void Initialize(void);
    void Resize(int, int);
    void Render(void);
    void uninitialize(void);
    
    //variable declarations
    int winWidth   = giWindowWidth;
    int winHeight  = giWindowHeight;
    bool bDone     = false;
    
    //code
    CreateWindow();
    
    Initialize();
    
    //Message Loop
    XEvent event;
    KeySym keysym;
    
    while(bDone == false)
    {
        while(XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);
            switch(event.type)
            {
                case MapNotify:
                    break;
            
                case KeyPress:
                    keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                    switch(keysym)
                    {
                        case XK_Escape:
                            bDone = true;
                            break;
                        
                        case XK_F:
                        case XK_f:
                            if(gbFullscreen == false)
                            {
                                ToggleFullscreen();
                                gbFullscreen = true;
                            }
                            else
                            {
                                ToggleFullscreen();
                                gbFullscreen = false;
                            }
                            break;
                    
                        case XK_L:
                        case XK_l:
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
                            
                        case XK_T:
                        case XK_t:
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
                            
                        case XK_A:
                        case XK_a:
                            if(gbAnimate == false)
                            {
                                gbAnimate = true;
                            }
                            else
                            {
                                gbAnimate = false;
                            }
                            break;
                            
                        default:
                            break;
                    }
                    break;
                        
                case ButtonPress:
                    switch(event.xbutton.button)
                    {
                        case 1:
                            break;
                            
                        case 2:
                            break;
                            
                        case 3:
                            break;
                            
                        default:
                            break;
                    }
                    break;
                        
                case MotionNotify:
                    break;
                    
                case ConfigureNotify:
                    winWidth = event.xconfigure.width;
                    winHeight = event.xconfigure.height;
                    Resize(winWidth, winHeight);
                    break;
                        
                case Expose:
                    break;
                    
                case DestroyNotify:
                    break;
                    
                case 33:
                    bDone = true;
                    break;
                    
                default:
                    break;
            }
        }
        
        Render();
    }
    
    uninitialize();
    
    return (0);
}

void CreateWindow(void)
{
    //function prototypes
    void uninitialize(void);
    
    //variable declarations
    XSetWindowAttributes winAttribs;
    int defaultScreen;
    int styleMask;
    
    static int frameBufferAttributes[] = { GLX_DOUBLEBUFFER, True,
                                           GLX_RGBA,
                                           GLX_RED_SIZE,        8,
                                           GLX_GREEN_SIZE,      8,
                                           GLX_BLUE_SIZE,       8,
                                           GLX_ALPHA_SIZE,      8,
                                           GLX_DEPTH_SIZE,     24,
                                           None };
    
    //code
    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        printf("ERROR : Unable To Open X Display.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    defaultScreen = XDefaultScreen(gpDisplay);
    
    gpXVisualInfo = (XVisualInfo*)malloc(sizeof(XVisualInfo));
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR : Unable To Allocate Memory For Visual Info.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
    if(gpXVisualInfo == NULL)
    {
        printf("ERROR : Unable To Get A Viusal.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    winAttribs.border_pixel       = 0;
    winAttribs.background_pixmap  = 0;
    winAttribs.colormap           = XCreateColormap(gpDisplay, 
                                                    RootWindow(gpDisplay, gpXVisualInfo->screen),
                                                    gpXVisualInfo->visual, 
                                                    AllocNone);
    winAttribs.background_pixel   = BlackPixel(gpDisplay, defaultScreen);
    winAttribs.event_mask         = ExposureMask | VisibilityChangeMask | ButtonPressMask |
                                    KeyPressMask | PointerMotionMask | StructureNotifyMask;
                                    
    styleMask                     = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
    gColormap                     = winAttribs.colormap;
    
    gWindow = XCreateWindow(gpDisplay, 
                            RootWindow(gpDisplay, gpXVisualInfo->screen),
                            0,
                            0,
                            giWindowWidth,
                            giWindowHeight,
                            0,
                            gpXVisualInfo->depth,
                            InputOutput,
                            gpXVisualInfo->visual,
                            styleMask,
                            &winAttribs);
    if(!gWindow)
    {
        printf("ERROR : Failed To Create Main Window.\nExiting Now...\n");
        uninitialize();
        exit(1);
    }
    
    XStoreName(gpDisplay, gWindow, "OpenGL : Mesh Loading");

    Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);
    
    XMapWindow(gpDisplay, gWindow);
}

void ToggleFullscreen(void)
{
    //variable declaration
    Atom wm_state;
    Atom fullscreen;
    XEvent xev = {0};
    
    //code
    wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
    memset(&xev, 0, sizeof(xev));
    
    xev.type                  = ClientMessage;
    xev.xclient.window        = gWindow;
    xev.xclient.message_type  = wm_state;
    xev.xclient.format        = 32;
    xev.xclient.data.l[0]     = gbFullscreen ? 0 : 1;
    
    fullscreen                = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[1]     = fullscreen;
    
    XSendEvent(gpDisplay, 
               RootWindow(gpDisplay, gpXVisualInfo->screen),
               False,
               StructureNotifyMask,
               &xev);
}

void Initialize(void)
{
    //function declarations
    void Resize(int, int);
    GLuint LoadBitmapAsTexture(const char* path);
    void LoadMeshData(void);
    
    //code
    gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
    glXMakeCurrent(gpDisplay, gWindow, gGLXContext);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    //set up light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);
    
    monkeyhead_texture = LoadBitmapAsTexture("monkey.bmp");
    
    LoadMeshData();
    
    Resize(giWindowWidth, giWindowHeight);
}

GLuint LoadBitmapAsTexture(const char* path)
{
    //variable declarations
    int width, height;
    unsigned char* image_data = NULL;
    GLuint texture_id;
    
    //code
    image_data = SOIL_load_image(path, &width, &height, NULL, SOIL_LOAD_RGB);
    
    //generate texture object
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    //set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    //push the data to the texture memory
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    
    //release image
    SOIL_free_image_data(image_data);
    image_data = NULL;
    
    return (texture_id);
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
    void uninitialize(void);
    
    //variable declaraions
    char *sep_space = (char*)" ";                     //space separator for strtok 
    char *sep_fslash = (char*)"/";                    //forward slash seperator for  strtok
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
        printf("Error : failed to open MonkeyHead.obj file.\n");
        uninitialize();
        exit(1);
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
    printf("Meshfile name : MonkeyHead.obj\n");
    printf("Vertices   : %zu\n", gpVertices->size);
    printf("Tex-Coords : %zu\n", gpTexCoords->size);
    printf("Normals    : %zu\n", gpNormals->size);
    printf("Triangles  : %zu\n", gpFaceTriangles->size);
}

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

void Render(void)
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
        rotation += 0.5f;
        if(rotation >= 360.0f)
            rotation = 0.0f;
    }
    
    glXSwapBuffers(gpDisplay, gWindow);
}

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
    //function declaration
    void uninitialize(void);
    
    //code
    void *p = calloc(nr_elements, size_per_element);
    if(!p)
    {
        printf("Fatal Error : calloc() failed to allocate memory.\n");
        uninitialize();
        exit(1);
    }

    return (p);
}

void *xrealloc(void *p, size_t new_size)
{
    //function declaration
    void uninitialize(void);
    
    //code
    void *ptr = realloc(p, new_size);
    if(!ptr)
    {
        printf("Fatal Error : realloc() failed to allocate memory.\n");
        uninitialize();
        exit(1);
    }

    return (ptr);
}

void uninitialize(void)
{
    //variable declarations
    GLXContext currentGLXContext;
    
    //code
    glDeleteTextures(1, &monkeyhead_texture);
    
    //clean mesh data
    cleanVector2Df(&gpVertices);
    cleanVector2Df(&gpTexCoords);
    cleanVector2Df(&gpNormals);
    cleanVector2Di(&gpFaceTriangles);
    cleanVector2Di(&gpFaceTexCoords);
    cleanVector2Di(&gpFaceNormals);
    
    currentGLXContext = glXGetCurrentContext();
    if(currentGLXContext == gGLXContext)
    {
        glXMakeCurrent(gpDisplay, 0, 0);
    }
    
    if(gGLXContext)
    {
        glXDestroyContext(gpDisplay, gGLXContext);
    }
    
    if(gWindow)
    {
        XDestroyWindow(gpDisplay, gWindow);
    }
    
    if(gColormap)
    {
        XFreeColormap(gpDisplay, gColormap);
    }
    
    if(gpXVisualInfo)
    {
        free(gpXVisualInfo);
        gpXVisualInfo = NULL;
    }
    
    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }
}





