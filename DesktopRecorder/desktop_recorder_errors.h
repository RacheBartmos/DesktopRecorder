#ifndef ERRORS_H
#define ERRORS_H
#include <string>
#ifdef WIN32
#include <Windows.h>
#endif
enum All_ERRORS{
    //normal
    ERROR_NO = 0,
    NORMAL_ERROR,
    NORMAL_UNSUPPORT,
    NORMAL_INVALID_CONTEXT,
    NORMAL_NEED_INIT,
    NORMAL_TIMEOUT,
    NORMAL_ALLOCATE_FAILED,

    //GDI_
    R_GDI_GET_DC_FAILED,
    R_GDI_CREATE_DC_FAILED,
    R_GDI_CREATE_BMP_FAILED,
    R_GDI_BITBLT_FAILED,
    R_GDI_GET_DIBITS_FAILED,

    //D3D_
    R_D3D_LOAD_FAILED,
    R_D3D_GET_PROC_FAILED,
    R_D3D_CREATE_DEVICE_FAILED,
    R_D3D_QUERYINTERFACE_FAILED,
    R_D3D_CREATE_VERTEX_SHADER_FAILED,
    R_D3D_CREATE_INLAYOUT_FAILED,
    R_D3D_CREATE_PIXEL_SHADER_FAILED,
    R_D3D_CREATE_SAMPLERSTATE_FAILED,

    //DXGI_
    R_DXGI_GET_PROC_FAILED,
    R_DXGI_GET_ADAPTER_FAILED,
    R_DXGI_GET_FACTORY_FAILED,
    R_DXGI_FOUND_ADAPTER_FAILED,

    //DUP_
    R_DUP_ATTATCH_FAILED,
    R_DUP_QI_FAILED,
    R_DUP_GET_PARENT_FAILED,
    R_DUP_ENUM_OUTPUT_FAILED,
    R_DUP_DUPLICATE_MAX_FAILED,
    R_DUP_DUPLICATE_FAILED,
    R_DUP_RELEASE_FRAME_FAILED,
    R_DUP_ACQUIRE_FRAME_FAILED,
    R_DUP_QI_FRAME_FAILED,
    R_DUP_CREATE_TEXTURE_FAILED,
    R_DUP_QI_DXGI_FAILED,
    R_DUP_MAP_FAILED,
    R_DUP_GET_CURSORSHAPE_FAILED,

    ERROR_MAX
};

static const char *ERRORS_STR[] = {
    "no error",                             //NO_ERROR = 0
    "error",                                //NORMAL_ERROR
    "not support for now",                  //NORMAL_UNSUPPORT
    "invalid context",                      //NORMAL_INVALID_CONTEXT
    "need init first",                      //NORMAL_NEED_INIT
    "operation timeout",                    //NORMAL_TIMEOUT
    "allocate memory failed",               //NORMAL_ALLOCATE_FAILED

    "gdi get dc failed",                    //R_GDI_GET_DC_FAILED
    "gdi create dc failed",                 //R_GDI_CREATE_DC_FAILED
    "gdi create bmp failed",                //R_GDI_CREATE_BMP_FAILED
    "gdi bitblt failed",                    //R_GDI_BITBLT_FAILED
    "gid geet dibbits failed",              //R_GDI_GET_DIBITS_FAILED

    "d3d11 library load failed",            //R_D3D_LOAD_FAILED
    "d3d11 proc get failed",                //R_D3D_GET_PROC_FAILED
    "d3d11 create device failed",           //R_D3D_CREATE_DEVICE_FAILED
    "d3d11 query interface failed",         //R_D3D_QUERYINTERFACE_FAILED
    "d3d11 create vertex shader failed",    //R_D3D_CREATE_VERTEX_SHADER_FAILED
    "d3d11 create input layout failed",     //R_D3D_CREATE_INLAYOUT_FAILED
    "d3d11 create pixel shader failed",     //R_D3D_CREATE_PIXEL_SHADER_FAILED
    "d3d11 create sampler state failed",    //R_D3D_CREATE_SAMPLERSTATE_FAILED

    "dxgi get proc address failed",         //R_DXGI_GET_PROC_FAILED
    "dxgi get adapter failed",              //R_DXGI_GET_ADAPTER_FAILED
    "dxgi get factory failed",              //R_DXGI_GET_FACTORY_FAILED
    "dxgi specified adapter not found",     //R_DXGI_FOUND_ADAPTER_FAILED

    "duplication attatch desktop failed",   //R_DUP_ATTATCH_FAILED
    "duplication query interface failed",   //R_DUP_QI_FAILED
    "duplication get parent failed",        //R_DUP_GET_PARENT_FAILED
    "duplication enum ouput failed",        //R_DUP_ENUM_OUTPUT_FAILED
    "duplication duplicate unavailable",    //R_DUP_DUPLICATE_MAX_FAILED
    "duplication duplicate failed",         //R_DUP_DUPLICATE_FAILED
    "duplication release frame failed",     //R_DUP_RELEASE_FRAME_FAILED
    "duplication acquire frame failed",     //R_DUP_ACQUIRE_FRAME_FAILED
    "duplication qi frame failed",          //R_DUP_QI_FRAME_FAILED
    "duplication create texture failed",    //R_DUP_CREATE_TEXTURE_FAILED
    "duplication dxgi qi failed",           //R_DUP_QI_DXGI_FAILED
    "duplication map rects failed",         //R_DUP_MAP_FAILED
    "duplication get cursor shape failed"   //R_DUP_GET_CURSORSHAPE_FAILE
};

#define err2str(e) e < ERROR_MAX ? ERRORS_STR[e] : "unknown"

namespace system_error {
static const std::string& error2str(unsigned long error) {
#ifdef WIN32
    DWORD system_locale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    HLOCAL local_buf = nullptr;
    BOOL ret = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, error, system_locale,(PSTR) &local_buf, 0, NULL);
    if (!ret) {
        HMODULE hnetmsg = LoadLibraryEx("netmsg.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
        if (hnetmsg != nullptr) {
            ret = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hnetmsg, error, system_locale, (PSTR)&local_buf, 0, NULL);
            FreeLibrary(hnetmsg);
        }
    }
    std::string error_str;
    if (ret) {
        error_str = (LPCTSTR)LocalLock(local_buf);
        LocalFree(local_buf);
    }
    return error_str;
#endif
}
};
#endif // ERRORS_H
