//headers
#include <windows.h>

//function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//variable declaration
bool gbFullscreen = true;

//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //variable declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyApp");

    //variables for fullscreen
    DEVMODE devModeScreen = { sizeof(DEVMODE) };
    RECT windowRect;
    DWORD windowStyle;
    DWORD extendedWindowStyle;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int bits = 32;

    windowRect.top = 0;
    windowRect.left = 0;
    windowRect.right = screenWidth;
    windowRect.bottom = screenHeight;

    //code
    //initialization of WNDCLASSEX
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_VREDRAW | CS_HREDRAW;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;
    wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

    //register above class
    RegisterClassEx(&wndclass);

    //fullscreen
    if(gbFullscreen == true)
    {
        devModeScreen.dmSize = sizeof(DEVMODE);
        devModeScreen.dmPelsWidth = screenWidth;
        devModeScreen.dmPelsHeight = screenHeight;
        devModeScreen.dmBitsPerPel = bits;
        devModeScreen.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

        if(ChangeDisplaySettings(&devModeScreen, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            MessageBox(NULL, "Display mode failed", NULL, MB_OK);
            gbFullscreen = false;
        }
    }
    
    if(gbFullscreen == true)
    {
        extendedWindowStyle = WS_EX_APPWINDOW;
        windowStyle = WS_POPUP;
        ShowCursor(false);
    }
    else
    {
        extendedWindowStyle = NULL;
        windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    }

    AdjustWindowRectEx(&windowRect, windowStyle, NULL, extendedWindowStyle);

    //create window
    hwnd = CreateWindow(szAppName,
        TEXT("My Application : Direct Fullscreen"),
        windowStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0,
        0,
        screenWidth,
        screenHeight,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, SW_MAXIMIZE);
    UpdateWindow(hwnd);

    //message loop
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //code
    switch(iMsg)
    {
        case WM_KEYDOWN:
            switch(wParam)
            {
                case 0x5B:
                    //fall through
                case 0x5C:
                    ShowWindow(hwnd, SW_MINIMIZE);
                    UpdateWindow(hwnd);
                    break;

                case 0x1B:
                    PostQuitMessage(0);
                    break;
                
                default:
                    break;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}


