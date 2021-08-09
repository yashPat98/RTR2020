//headers
#include <stdio.h>
#include <d3d11.h>
#include <math.h>

//library linking 
#pragma comment(lib, "d3d11.lib")           
#pragma comment(lib, "dxgi.lib")            

int main(void)
{
    //variable declarations
    IDXGIFactory *pIDXGIFactory = NULL;
    IDXGIAdapter *pIDXGIAdapter = NULL;
    DXGI_ADAPTER_DESC dxgiAdapterDesc;
    HRESULT hr;
    char str[255];

    //code
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
    if(FAILED(hr))
    {
        printf("Error : CreateDXGIFactory() failed.\n");
        goto cleanup;
    }

    if(pIDXGIFactory->EnumAdapters(1, &pIDXGIAdapter) == DXGI_ERROR_NOT_FOUND)
    {   
        printf("Error : DXGIAdapter cannot be found.\n");
        goto cleanup;
    }

    ZeroMemory((void*)&dxgiAdapterDesc, sizeof(DXGI_ADAPTER_DESC));

    hr = pIDXGIAdapter->GetDesc(&dxgiAdapterDesc);
    if(FAILED(hr))
    {
        printf("Error : dxgiAdapterDesc cannot be found.\n");
        goto cleanup;
    }

    WideCharToMultiByte(CP_ACP, 0, dxgiAdapterDesc.Description, 255, str, 255, NULL, NULL);
    printf("Graphic Card Name : %s\n", str);
    printf("Graphic Card VRam in bytes : %I64d bytes\n", (__int64)dxgiAdapterDesc.DedicatedVideoMemory);
    printf("Graphic Card VRam in GB : %d\n", (int)ceil(dxgiAdapterDesc.DedicatedVideoMemory / 1024.0 / 1024.0 / 1024.0));

    cleanup :
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

    return (0);
}

