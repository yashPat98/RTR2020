//headers
#include <windows.h>

//macros
#define WIDTH  800
#define HEIGHT 600

//function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //variable declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyApp");
    int xCord, yCord;

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

    //get x and y coordinates for window (left-top)
    xCord = ((GetSystemMetrics(SM_CXSCREEN) - WIDTH) / 2);
    yCord = ((GetSystemMetrics(SM_CYSCREEN) - HEIGHT) / 2);

    //create window
    hwnd = CreateWindow(szAppName,
        TEXT("My Application : WM_PAINT"),
        WS_OVERLAPPEDWINDOW,
        xCord,
        yCord,
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
    //variable declaration
    HDC hdc;
    RECT rect;
    TCHAR str[] = TEXT("Hello World !!!");

    //code
    switch(iMsg)
    {
        case WM_PAINT:
            GetClientRect(hwnd, &rect);
            hdc = GetDC(hwnd);
            SetBkColor(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(0, 255, 0));
            DrawText(hdc, str, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            ReleaseDC(hwnd, hdc);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
    }

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}
