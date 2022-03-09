#include "dxcutscreen.h"

DxCutScreen::DxCutScreen()
{

}

int DxCutScreen::init(HWND hWnd)
{
    int bRet = 0;
    HRESULT hr;
    do
    {
        //creating a D3D9 object
        g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if(g_pD3D == nullptr) {
            bRet = -1;
            break;
        }

        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp,sizeof (d3dpp));
        d3dpp.Windowed= true;
        d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = hWnd;

        hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,hWnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dpp,&g_pd3dDevice);
        if(hr != D3D_OK || g_pd3dDevice == nullptr){
            bRet = -2;
            break;
        }

        D3DDISPLAYMODE ddm = {0};

        //Retrieves the display mode
        hr = g_pd3dDevice->GetDisplayMode(0,&ddm);
        if(hr != D3D_OK){
            bRet = -5;
            break;
        }

        width = ddm.Width;
        height = ddm.Height;
        //Creat an off-screen surface.
        hr = g_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width,ddm.Height,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&g_pD3DSurface,nullptr);
        if(hr != D3D_OK){
            bRet = -6;
            break;
        }
        bRet = 1;
        break;
    }while (false);
    return bRet;
}

void *DxCutScreen::Capture()
{
    if(!(g_pd3dDevice && g_pD3DSurface)){
        return nullptr;
    }
    HRESULT hr;
    hr = g_pd3dDevice->GetFrontBufferData(0,g_pD3DSurface);
    if(hr != D3D_OK){
        return nullptr;
    }
    D3DLOCKED_RECT d3d_rect;
    hr = g_pD3DSurface->LockRect(&d3d_rect,nullptr,D3DLOCK_DONOTWAIT);
    if(FAILED(hr)){
        return nullptr;
    }
    void* buf = d3d_rect.pBits;
    g_pD3DSurface->UnlockRect();
    return buf;
}
