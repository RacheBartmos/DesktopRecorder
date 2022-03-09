#ifndef DXCUTSCREEN_H
#define DXCUTSCREEN_H

#include <d3d9.h>
#include <Windows.h>
#include <iostream>

class DxCutScreen
{
private:
    LPDIRECT3D9 g_pD3D;
    LPDIRECT3DDEVICE9 g_pd3dDevice;
    LPDIRECT3DSURFACE9 g_pD3DSurface;
public:
    uint32_t width;
    uint32_t height;
public:
    DxCutScreen();
    int init(HWND hWnd);
    void* Capture();
};

#endif // DXCUTSCREEN_H
