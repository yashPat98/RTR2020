//headers
#include <windows.h>               //standard windows header
#include <stdio.h>                 //standard C io header 
#include <math.h>                  //standard C math header 
#include <d3d11.h>                 //Direct3D 11 header
#include "RESOURCES.h"             //Resources header

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

//symbolic constants
#define WIN_WIDTH  800             //initial width of window  
#define WIN_HEIGHT 600             //initial height of window

#define VK_F       0x46            //virtual key code of F key
#define VK_f       0x60            //virtual key code of f key

//callback procedure declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//global variables
HWND   ghwnd  = NULL;              //handle to a window
DWORD dwStyle = NULL;              //window style
WINDOWPLACEMENT wpPrev;            //structure for holding previous window position

bool gbActiveWindow = false;       //flag indicating whether window is active or not
bool gbFullscreen = false;         //flag indicating whether window is fullscreen or not

FILE*  gpFile = NULL;              //log file handle
char gLogFileName[] = "log.txt";   //log file name

IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
float gClearColor[4];

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    HRESULT Initialize(void);                              //initialize OpenGL state machine
    void Display(void);                                    //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                   //structure holding window class attributes
    MSG msg;                                               //structure holding message attributes
    HWND hwnd;                                             //handle to a window
    TCHAR szAppName[] = TEXT("Direct3D11");                //name of window class
    HRESULT hr = S_OK;

    int cxScreen, cyScreen;                                //screen width and height for centering window
    int init_x, init_y;                                    //top-left coordinates of centered window
    bool bDone = false;                                    //flag indicating whether or not to exit from game loop

    //code
    //create/open  'log.txt' file
    if(fopen_s(&gpFile, gLogFileName, "w") != 0)
    {
        TCHAR str[255];
        wsprintf(str, TEXT("Failed to open %s file"), gLogFileName);
        MessageBox(NULL, str, TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "--------------------------------------------------------------------------\n");
        fprintf(gpFile, "-> Program started successfully\n");
        fclose(gpFile);
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
            TEXT("Direct3D11 : Bluescreen"),              //window caption
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
    hr = Initialize();
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : Initialize() failed.\n");
        fclose(gpFile);
        DestroyWindow(ghwnd);
    }

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
    void ToggleFullscreen(void);                //toggle window between fullscreen and previous position 
    HRESULT Resize(int, int);                   //handle window resize event
    void Cleanup(void);                         //release resources  

    //variable declarations
    HRESULT hr = S_OK;

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
            if(gpID3D11DeviceContext)
            {
                hr = Resize(LOWORD(lParam), HIWORD(lParam));
                if(FAILED(hr))
                {
                    fopen_s(&gpFile, gLogFileName, "a+");
                    fprintf(gpFile, "Error : Resize() failed.\n");
                    fclose(gpFile);
                    DestroyWindow(ghwnd);
                }
            }
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
        
        case WM_CLOSE:                           //event : window is closed from sysmenu or close button
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            Cleanup();
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

HRESULT Initialize(void)
{
    //function declarations
    void Cleanup(void);
    HRESULT printD3DInfo(void);
    HRESULT Resize(int width, int height);

    //variable declarations
    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    
    D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;
    D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
    D3D_DRIVER_TYPE d3dDriverType;
    
    UINT numFeatureLevels = 1;
    UINT numDriverTypes = 0;
    UINT createDeviceFlags = 0;

    HRESULT hr = S_OK;
    
    //code
    hr = printD3DInfo();
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : printD3DInfo() failed.\n");
        fclose(gpFile);
        return (hr);
    }

    //initialize dxgiSwapChainDesc structure
    ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    dxgiSwapChainDesc.BufferCount = 1;
    dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
    dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
    dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.OutputWindow = ghwnd;
    dxgiSwapChainDesc.SampleDesc.Count = 1;
    dxgiSwapChainDesc.SampleDesc.Quality = 0;
    dxgiSwapChainDesc.Windowed = TRUE;

    //get total number of types of drivers
    numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

    //enumerate driver types to choose the best
    for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        d3dDriverType = d3dDriverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(
            NULL,
            d3dDriverType,
            NULL,
            createDeviceFlags,
            &d3dFeatureLevel_required,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &dxgiSwapChainDesc,
            &gpIDXGISwapChain,
            &gpID3D11Device,
            &d3dFeatureLevel_acquired,
            &gpID3D11DeviceContext
        );

        if(SUCCEEDED(hr))
            break;
    }

    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : D3D11CreateDeviceAndSwapChain() failed.\n");
        fclose(gpFile);
        return (hr);
    }
    else
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "-> Driver Info\n");
        fprintf(gpFile, "Driver Type    : ");
        if(d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
        {
            fprintf(gpFile, "Hardware Type.\n");
        }
        else if(d3dDriverType == D3D_DRIVER_TYPE_WARP)
        {
            fprintf(gpFile, "Warp Type.\n");
        }
        else if(d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
        {
            fprintf(gpFile, "Reference Type.\n");
        }
        else
        {
            fprintf(gpFile, "Unknown Type.\n");
        }

        fprintf(gpFile, "Feature Level  : ");
        if(d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_11_0)
        {
            fprintf(gpFile, "11.0\n");
        }
        else if(d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_1)
        {
            fprintf(gpFile, "10.1\n");
        }
        else if(d3dFeatureLevel_acquired == D3D_FEATURE_LEVEL_10_0)
        {
            fprintf(gpFile, "10.0\n");
        }
        else
        {
            fprintf(gpFile, "Unknown.\n");
        }
        fclose(gpFile);
    }

    //set clear color
    gClearColor[0] = 0.0f;
    gClearColor[1] = 0.0f;
    gClearColor[2] = 1.0f;
    gClearColor[3] = 1.0f;

    //warm-up call
    hr = Resize(WIN_WIDTH, WIN_HEIGHT);
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : Resize() failed.\n");
        fclose(gpFile);
        return (hr);
    }

    return (hr);
}

HRESULT printD3DInfo(void)
{
    //variable declarations
    IDXGIFactory *pIDXGIFactory = NULL;
    IDXGIAdapter *pIDXGIAdapter = NULL;
    DXGI_ADAPTER_DESC dxgiAdapterDesc;
    HRESULT hr;
    char str[255];

    //code 
    //initialize IDXGIFactory
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : CreateDXGIFactory() failed.\n");
        fclose(gpFile);
    }
    else
    {
        //Get IDXGIAdapter (Device Interface) from IDXGIFactory
        if(pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter) == DXGI_ERROR_NOT_FOUND)
        {
            fopen_s(&gpFile, gLogFileName, "a+");
            fprintf(gpFile,"Error : pIDXGIFactory->EnumAdapters() failed.\n");
            fclose(gpFile);
        }
        else
        {
            //zero out DXGI_ADAPTER_DESC structure
            ZeroMemory((void*)&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));
            
            //get DXGI_ADAPTER_DESC from IDXGIAdapter (Device Interface)
            hr = pIDXGIAdapter->GetDesc(&dxgiAdapterDesc);
            if(FAILED(hr))
            {
                fopen_s(&gpFile, gLogFileName, "a+");
                fprintf(gpFile, "Error : pIDXGIAdapter->GetDesc() failed.\n");
                fclose(gpFile);
            }
            else
            {
                //log the device info from DXGI_ADAPTER_DESC
                WideCharToMultiByte(CP_ACP, 0, dxgiAdapterDesc.Description, 255, str, 255, NULL, NULL);

                fopen_s(&gpFile, gLogFileName, "a+");
                fprintf(gpFile, "--------------------------------------------------------------------------\n");
                fprintf(gpFile, "-> Device Info\n");
                fprintf(gpFile, "Name           : %s\n", str);
                fprintf(gpFile, "VRAM           : %d MB\n", (int)ceil(dxgiAdapterDesc.DedicatedVideoMemory / 1024.0 / 1024.0));
                fprintf(gpFile, "--------------------------------------------------------------------------\n");
                fclose(gpFile);
            }
        }
    }

    //cleanup
    if(pIDXGIAdapter)
    {
        pIDXGIAdapter->Release();
        pIDXGIAdapter = NULL;
    }

    if(pIDXGIFactory)
    {
        pIDXGIFactory->Release();
        pIDXGIFactory = NULL;
    }

    return (hr);
}

HRESULT Resize(int width, int height)
{
    //variable declarations
    ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;
    D3D11_VIEWPORT d3dViewport;
    HRESULT hr = S_OK;

    //code
    //free size dependant resources
    if(gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    //resize swap chain buffers
    hr = gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : gpIDXGISwapChain->ResizeBuffers() failed.\n");
        fclose(gpFile);
        return (hr);
    }

    //get back buffer from swap chain
    hr = gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2D_BackBuffer);
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : gpIDXGISwapChain->GetBuffer() failed.\n");
        fclose(gpFile);
        return (hr);
    }

    //create render target view for resized buffer
    hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);
    if(FAILED(hr))
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "Error : gpID3D11Device->CreateRenderTargetView() failed.\n");
        fclose(gpFile);
        return (hr);
    }

    pID3D11Texture2D_BackBuffer->Release();
    pID3D11Texture2D_BackBuffer = NULL;

    //set render target view as output merger render target
    gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

    //set viewport
    d3dViewport.TopLeftX = 0.0f;
    d3dViewport.TopLeftY = 0.0f;
    d3dViewport.Width = (float)width;
    d3dViewport.Height = (float)height;
    d3dViewport.MinDepth = 0.0f;
    d3dViewport.MaxDepth = 1.0f;
    gpID3D11DeviceContext->RSSetViewports(1, &d3dViewport);

    return (hr);
}

void Display(void)
{
    //code
    //clear color buffer
    gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gClearColor);

    //swap buffers
    gpIDXGISwapChain->Present(0, 0);
}

void Cleanup(void)
{
    //code
    //if window is in fullscreen mode toggle
    if(gbFullscreen == true)
    {
        ToggleFullscreen();
    }

    //release D3D11 and DXGI resources
    if(gpID3D11RenderTargetView)
    {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    if(gpIDXGISwapChain)
    {
        gpIDXGISwapChain->Release();
        gpIDXGISwapChain = NULL;
    }

    if(gpID3D11DeviceContext)
    {
        gpID3D11DeviceContext->Release();
        gpID3D11DeviceContext = NULL;
    }

    if(gpID3D11Device)
    {
        gpID3D11Device->Release();
        gpID3D11Device = NULL;
    }

    //close the log file
    if(gpFile)
    {
        fopen_s(&gpFile, gLogFileName, "a+");
        fprintf(gpFile, "--------------------------------------------------------------------------\n");
        fprintf(gpFile, "-> Program completed successfully.\n");
        fprintf(gpFile, "--------------------------------------------------------------------------\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
