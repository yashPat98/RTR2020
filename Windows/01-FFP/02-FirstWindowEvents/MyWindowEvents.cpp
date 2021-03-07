//Headers
#include <windows.h>

//global function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //Variable declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyApp");

    //code
    //initialization of WNDCLASSEX
    wndclass.cbSize             = sizeof(WNDCLASSEX);
    wndclass.style              = CS_HREDRAW | CS_VREDRAW;
    wndclass.cbClsExtra         = 0;
    wndclass.cbWndExtra         = 0;
    wndclass.lpfnWndProc        = WndProc;
    wndclass.hInstance          = hInstance;
    wndclass.hIcon              = LoadIcon(NULL, IDI_APPLICATION);          //Taskbar Icon
    wndclass.hCursor            = LoadCursor(NULL, IDC_ARROW);             
    wndclass.hbrBackground      = (HBRUSH)GetStockObject(DKGRAY_BRUSH);     
    wndclass.lpszClassName      = szAppName;
    wndclass.lpszMenuName       = NULL;
    wndclass.hIconSm            = LoadIcon(NULL, IDI_APPLICATION);          //Top-left Window icon 

    //Register above class
    RegisterClassEx(&wndclass);

    //create window
    hwnd = CreateWindow(szAppName, 
        TEXT("My Application"),
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        800,
        600,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    //message loop
    //GetMessage returns false only for WM_QUIT
    while(GetMessage(&msg, NULL, 0, 0))        //LPMSG, HWND, UINT, UINT
    {
        TranslateMessage(&msg);                //Translate message to low level 
        DispatchMessage(&msg);                 //Call WndProc with parameters from struct msg
    }

    return ((int)msg.wParam);
}

//WndProc definition
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch(iMsg)
    {
        case WM_CREATE:
            MessageBox(hwnd, TEXT("WM_CREATE is recieved"), TEXT("MyMessage"), MB_OK);        //creates message window using predefined wndclass
            break;
        case WM_LBUTTONDOWN:
            MessageBox(hwnd, TEXT("WM_LBUTTONDOWN is recieved"), TEXT("MyMessage"), MB_OK);
            break;
        case WM_RBUTTONDOWN:
            MessageBox(hwnd, TEXT("WM_RBUTTONDOWN is recieved"), TEXT("MyMessage"), MB_OK);
            break;
        case WM_KEYDOWN:
            MessageBox(hwnd, TEXT("WM_KEYDOWN is recieved"), TEXT("MyMessage"), MB_OK);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);                 //posts msg with iMsg as WM_QUIT and wParam as 0 in message pool 
            break;
    }
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}
