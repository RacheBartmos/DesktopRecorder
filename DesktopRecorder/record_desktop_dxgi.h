#ifndef RECORD_DESKTOP_DXGI_H
#define RECORD_DESKTOP_DXGI_H

#include "record_desktop.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <list>

using namespace D3D_Helper;
class Record_Desktop_DXGI : public Record_Desktop
{
    typedef struct _PTR_INFO
    {
        _Field_size_bytes_(BufferSize) BYTE* buff;
        DXGI_OUTDUPL_POINTER_SHAPE_INFO shape;
        POINT position;
        bool visible;
        UINT size;
        UINT output_index;
        LARGE_INTEGER pre_timestamp;
    }DUPLICATION_CURSOR_INFO;
public:
	Record_Desktop_DXGI();
    ~Record_Desktop_DXGI();
    virtual int init(const RECORD_DESKTOP_RECT &rect, const int fps);
    virtual int start();
    virtual int stop();
protected:
    virtual void clean_up() override;
private:
    int get_dst_adapter(IDXGIAdapter **adapter);
    int create_d3d_device(IDXGIAdapter *adapter,ID3D11Device **device);
    int init_d3d11();
    void clean_d3d11();
    int init_duplication();
    int free_duplicated_frame();
    void clean_duplication();
    bool attatch_desktop();
    int get_desktop_image(DXGI_OUTDUPL_FRAME_INFO *frame_info);
    int get_desktop_cursor(const DXGI_OUTDUPL_FRAME_INFO *frame_info);
    void draw_cursor();
    void do_sleep(int64_t dur, int64_t pre, int64_t now);
    void record_func();
private:
    uint8_t *_buffer;
    uint32_t _buffer_size;
    uint32_t _width, _height;
    HMODULE _d3d11, _dxgi;
    ID3D11Device* _d3d_device;
    ID3D11DeviceContext* _d3d_ctx;
    ID3D11VertexShader* _d3d_vshader;
    ID3D11PixelShader* _d3d_pshader;
    ID3D11InputLayout* _d3d_inlayout;
    ID3D11SamplerState* _d3d_samplerlinear;
    IDXGIOutputDuplication *_duplication;
    ID3D11Texture2D *_image;
    DXGI_OUTPUT_DESC _output_des;
    int _output_index;
    DUPLICATION_CURSOR_INFO _cursor_info;
};

#endif
