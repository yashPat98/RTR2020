//headers
#include <windows.h>

//macros
#define WIDTH  800      //width of window
#define HEIGHT 600      //height of window

//global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) 
{
    //variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyApp");
    RECT rect;
    LONG cWidth, cHeight;

    //code
    //initialization of WNDCLASSEX 
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
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

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
    cWidth = rect.right - rect.left;
    cHeight = rect.bottom - rect.top;

    //create window
    hwnd = CreateWindow(szAppName,
        TEXT("My Application"),
        WS_OVERLAPPEDWINDOW,
        (cWidth / 2) - (WIDTH / 2),
        (cHeight / 2) - (HEIGHT / 2),
        WIDTH,
        HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    //message loop
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return ((int)msg.wParam);
}

//function definition
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //code
    switch(iMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

