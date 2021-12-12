#ifndef RECORD_DXGI_H
#define RECORD_DXGI_H

#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi.h>

#include <dxgi.h>
#include <d3d11.h>

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"Shcore.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"windowscodecs.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "dxguid.lib")

#define LEN(e) (sizeof(e)/sizeof(e[0]))

extern "C"{
struct DXGI_FrameCapturer;

struct cursorData{
    HCURSOR current_cursor;
    POINT cursor_pos;
    long x_hotspot;
    long y_hotspot;
    bool visible;

    uint32_t last_cx;
    uint32_t last_cy;
};

struct DXGI_FrameCapturer
{
    DXGI_FrameCapturer();
    ID3D11Device            *device=nullptr;
    ID3D11DeviceContext     *device_context=nullptr;
    IDXGIOutputDuplication  *output_duplication=nullptr;
    IDXGIFactory1           *dxgi_factory = nullptr;
    HWND capture_window;

    int captured_display_left;
    int captured_display_top;
    int captured_display_right;
    int captured_display_bottom;

    int capture_window_left;
    int capture_window_top;
    int capture_window_right;
    int capture_window_bottom;

    ID3D11Texture2D *capture_texture=nullptr;
    ID3D11Texture2D *region_copy_texture=nullptr;
    IDXGISurface *region_copy_surface=nullptr;

    int h;
    int w;
    int x;
    int y;
    int width;
    int height;
    int waitinit;
    int triggerable;
    unsigned int cnt;
    unsigned int *tsz;
    unsigned char **bf;

    /*
     *
     *
     * #include <dxgi.h>
     * #include <d3d11.h>
     *
     *
     *
     *   GUID IIDXGISurface = { 0xcafcb56c, 0x6ac3, 0x4889, 0xbf, 0x47, 0x9e, 0x23, 0xbb, 0xd2, 0x60, 0xec}; //IID_IDXGISurface
     *   GUID IIDXGIFactory1 = { 0x770aae78, 0xf26f, 0x4dba, 0xa8, 0x29, 0x25, 0x3c, 0x83, 0xd1, 0xb3, 0x87 }; //IID_IDXGIFactory1
     *   GUID IID3D11Texture2D = { 0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48, 0x95, 0x35, 0xd3, 0x4f, 0x9c }; //IID_ID3D11Texture2D
     */
    bool verbosity = false;
    uint32_t getLenght();
    uint32_t getHeight();
    uint32_t getWidth();

    bool captureStateInit(int x, int y, int w, int h);
};

static inline bool tryCaptureFrame(DXGI_FrameCapturer *capture,unsigned char* copy_to_buffer)
{
    DXGI_OUTDUPL_FRAME_INFO capture_frame_info;
    IDXGIResource *resource;
    HRESULT hr = S_OK;
    hr = capture->output_duplication->AcquireNextFrame(100, &capture_frame_info, &resource);
    if (FAILED(hr))
    {
        if (capture->verbosity) printf("Error: no new frames");
        capture->captureStateInit(capture->x, capture->y, capture->w, capture->h);
        return false;
    }

    resource->QueryInterface(IID_ID3D11Texture2D, reinterpret_cast<void**>(&capture->capture_texture));
    resource->Release();

    if (capture->capture_texture)
    {
        if (capture->verbosity) printf("texture captured ok\n");
        D3D11_TEXTURE2D_DESC capture_texture_desc;
        capture->capture_texture->GetDesc(&capture_texture_desc);

        D3D11_TEXTURE2D_DESC region_texture_desc;
        ZeroMemory(&region_texture_desc, sizeof(region_texture_desc));

        region_texture_desc.Width = capture->width;
        region_texture_desc.Height = capture->height;
        region_texture_desc.MipLevels = 1;
        region_texture_desc.ArraySize = 1;
        region_texture_desc.SampleDesc.Count = 1;
        region_texture_desc.SampleDesc.Quality = 0;
        region_texture_desc.Usage = D3D11_USAGE_STAGING;
        region_texture_desc.Format = capture_texture_desc.Format;
        region_texture_desc.BindFlags = 0;
        region_texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        region_texture_desc.MiscFlags = 0;

        hr = capture->device->CreateTexture2D(&region_texture_desc, nullptr, &capture->region_copy_texture);
        if (FAILED(hr))
        {
            if (capture->verbosity) printf("error - CreateTexture2d:%ld", hr);
            capture->capture_texture->Release();
            capture->captureStateInit(capture->x, capture->y, capture->w, capture->h);
            return false;
        }
        capture->capture_texture->Release();
        if (capture->verbosity) printf("texture created ok\n");
        // copy region of screen to texture
        D3D11_BOX source_region;
        source_region.left = capture->capture_window_left;
        source_region.right = capture->capture_window_right;
        source_region.top = capture->capture_window_top;
        source_region.bottom = capture->capture_window_bottom;
        source_region.front = 0;
        source_region.back = 1;
        capture->device_context->CopySubresourceRegion((ID3D11Resource *)(capture->region_copy_texture), 0, 0, 0, 0, (ID3D11Resource *)(capture->capture_texture), 0, &source_region);
        capture->region_copy_texture->QueryInterface(IID_IDXGISurface, reinterpret_cast<void**>(&capture->region_copy_surface));

        DXGI_MAPPED_RECT rect;
        if (capture->verbosity)printf("surface mapped ok\n");
        hr = capture->region_copy_surface->Map(&rect, DXGI_MAP_READ);
        if (FAILED(hr))
        {
            if (capture->verbosity) printf("error - region_copy_surface:%ld", hr);
            capture->region_copy_surface->Unmap();
            capture->region_copy_surface->Release();
            capture->region_copy_texture->Release();
            capture->captureStateInit(capture->x, capture->y, capture->w, capture->h);
            return false;
        }

        unsigned char *dest = copy_to_buffer;
        unsigned char *src = rect.pBits;
        int comp = 4;

        for (int row = 0; row < capture->height; row++)
        {
            memcpy(dest, src, static_cast<size_t>(capture->width * comp));
            dest += capture->width * comp;
            src += rect.Pitch;
        }

        if (capture->verbosity) printf("surface moved ok\n");

        capture->region_copy_surface->Unmap();
        capture->region_copy_surface->Release();
        capture->output_duplication->ReleaseFrame();
        capture->region_copy_texture->Release();
        if (capture->verbosity) printf("memory deallocated o\n");
        return true;
    }
    return false;
}

}

#endif // RECORD_DXGI_H
