#ifndef DEFINES_H
#define DEFINES_H
#include <Windows.h>
#include <iostream>
#include <list>
#include <dxgi1_2.h>

namespace System_Helper {
	static char system_path[260] = { 0 };
	static bool get_system_path() {
		if (strlen(system_path) == 0) {
			UINT ret = GetSystemDirectoryA(system_path, MAX_PATH);
			if (!ret) {
				printf("failed to get system directory :%lu", GetLastError());
				return false;
			}
		}
		return true;
	}
    HMODULE load_system_library(const char* name);
    void free_system_library(HMODULE handle);
};

namespace D3D_Helper {
	typedef HRESULT(WINAPI* DXGI_FUNC_CREATEFACTORY)(REFIID, IDXGIFactory1**);
	static std::list<IDXGIAdapter*> get_adapters(int* error, bool free_lib = false) {
		std::list<IDXGIAdapter*> adapters;
		*error = 0;
		HMODULE hdxgi = System_Helper::load_system_library("dxgi.dll");
		if (!hdxgi) {
			*error = -1;
			return adapters;
		}
		do {
			DXGI_FUNC_CREATEFACTORY create_factory = nullptr;
			create_factory = (DXGI_FUNC_CREATEFACTORY)GetProcAddress(hdxgi, "CreateDXGIFactory1");
			if (create_factory == nullptr) {
				*error = -1;
				break;
			}
			IDXGIFactory1* dxgi_factory = nullptr;
			HRESULT hr = create_factory(__uuidof(IDXGIFactory1), &dxgi_factory);
			if (FAILED(hr)) {
				*error = -1;
				break;
			}
			unsigned int i = 0;
			IDXGIAdapter* adapter = nullptr;
			while (dxgi_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
			{
				if (adapter) adapters.push_back(adapter);
				++i;
			}
			dxgi_factory->Release();
		} while (0);
		if (free_lib && hdxgi) System_Helper::free_system_library(hdxgi);
		return adapters;
	}
};

typedef enum {
	DT_DESKTOP_NO = 0,
	DT_DESKTOP_FFMPEG_GDI,
	DT_DESKTOP_FFMPEG_DSHOW,
	DT_DESKTOP_WIN_GDI,
	DT_DESKTOP_WIN_DUPLICATION,
}RECORD_DESKTOP_TYPES;

/*
* Record desktop data type
*/

typedef enum {
	AT_DESKTOP_NO = 0,
	AT_DESKTOP_RGBA,
	AT_DESKTOP_BGRA
}RECORD_DESKTOP_DATA_TYPES;

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
}RECORD_DESKTOP_RECT;

#endif // DEFINES_H
