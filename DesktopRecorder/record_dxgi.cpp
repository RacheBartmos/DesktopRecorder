#include "record_dxgi.h"

extern "C"{
uint32_t DXGI_FrameCapturer::getLenght()
{
    return this->height*this->width*4;
}

uint32_t DXGI_FrameCapturer::getHeight()
{
    return this->height;
}

uint32_t DXGI_FrameCapturer::getWidth()
{
    return this->width;
}

void DXGI_FrameCapturer::captureStateInit(int x, int y, int w, int h)
{
    if (this->dxgi_factory != nullptr)dxgi_factory->Release();
    if (this->device != nullptr)this->device->Release();
    if (this->device_context != nullptr)this->device_context->Release();
    if (this->output_duplication != nullptr)this->output_duplication->Release();

    HRESULT hr = CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void**>(&dxgi_factory));

    if (FAILED(hr))
    {
        printf("Error create dxgi factory: %ld",hr);
    }

    D3D_FEATURE_LEVEL supported_feature_levels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL fl;
    POINT pt;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, supported_feature_levels, LEN(supported_feature_levels), D3D11_SDK_VERSION, &this->device, &fl, &this->device_context);
    if (FAILED(hr))
    {
         printf("Error create d3d11 device: %ld",hr);
    }

    this->x = x;
    this->y = y;
    this->h = h;
    this->w = w;
    this->captured_display_left = 0;
    this->captured_display_top = 0;
    this->captured_display_right = 0;
    this->captured_display_bottom = 0;

    this->output_duplication = nullptr;
    this->capture_texture = nullptr;
    this->region_copy_texture = nullptr;
    this->region_copy_surface = nullptr;

    pt.x = (x + w) / 2;
    pt.y = (y + h) / 2;

    this->capture_window = WindowFromPoint(pt);

    this->capture_window_left = x;
    this->capture_window_top = y;
    this->capture_window_right = w;
    this->capture_window_bottom = h;

    // find the display that has the window on it.
    IDXGIAdapter1 *adapter;
    for (uint32_t adapter_index = 0; dxgi_factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND; adapter_index++)
    {
        // enumerate outputs
        IDXGIOutput *output;
        for (uint32_t output_index = 0; adapter->EnumOutputs(output_index, &output) != DXGI_ERROR_NOT_FOUND; output_index++)
        {
            DXGI_OUTPUT_DESC output_desc;
            output->GetDesc(&output_desc);
            if (output_desc.AttachedToDesktop)
            {
                if (output_desc.DesktopCoordinates.left <= this->capture_window_left && output_desc.DesktopCoordinates.right >= this->capture_window_right && output_desc.DesktopCoordinates.top <= this->capture_window_top && output_desc.DesktopCoordinates.bottom >= this->capture_window_bottom)
                {
                    this->captured_display_left = output_desc.DesktopCoordinates.left;
                    this->captured_display_right = output_desc.DesktopCoordinates.right;
                    this->captured_display_bottom = output_desc.DesktopCoordinates.bottom;
                    this->captured_display_top = output_desc.DesktopCoordinates.top;

                    IDXGIOutput1 *output1 = static_cast<IDXGIOutput1*>(output);
                    hr = output1->DuplicateOutput(static_cast<IUnknown *>(this->device), &this->output_duplication);
                    if (FAILED(hr))
                    {
                        captureStateInit(x,y,w,h);
                        //ShowError(QStringLiteral("Output Duplication Failed: %1").arg(hr));
                    }
                }
            }
            output->Release();
        }
        adapter->Release();
    }

    this->width = this->capture_window_right - this->capture_window_left;
    this->height = this->capture_window_bottom - this->capture_window_top;
}



DXGI_FrameCapturer::DXGI_FrameCapturer()
{
    captureStateInit(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
}

}
