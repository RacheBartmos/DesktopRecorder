#include "record_desktop_dxgi.h"
#include "desktop_recorder_errors.h"
#include "utils_string.h"

Record_Desktop_DXGI::Record_Desktop_DXGI()
{
	_data_type = RECORD_DESKTOP_DATA_TYPES::AT_DESKTOP_BGRA;
	_buffer = NULL;
	_buffer_size = 0;
	_d3d11 = nullptr;
	_dxgi = nullptr;
	_d3d_device = nullptr;
	_d3d_ctx = nullptr;
	_d3d_vshader = nullptr;
	_d3d_pshader = nullptr;
	_d3d_inlayout = nullptr;
	_d3d_samplerlinear = nullptr;
	_duplication = nullptr;
	_image = nullptr;
	_output_des = { 0 };
	_output_index = 0;
	ZeroMemory(&_cursor_info, sizeof(_cursor_info));
}


Record_Desktop_DXGI::~Record_Desktop_DXGI()
{
    //use derived class's virtual func instead of base class in destructor
    Record_Desktop_DXGI::stop();
    Record_Desktop_DXGI::clean_up();
}

int Record_Desktop_DXGI::init(const RECORD_DESKTOP_RECT& rect, const int fps)
{
	int error = ERROR_NO;
	if (_inited == true) return error;
	_fps = fps;
	_rect = rect;
	do {
        _d3d11 = System_Helper::load_system_library("d3d11.dll");
        _dxgi = System_Helper::load_system_library("dxgi.dll");
		if (!_d3d11 || !_dxgi) {
			error = R_D3D_LOAD_FAILED;
			break;
		}
		error = init_d3d11();
		if (error != ERROR_NO) break;
		_width = rect.right - rect.left;
		_height = rect.bottom - rect.top;
		_buffer_size = (_width * 32 + 31) / 32 * _height * 4;
		_buffer = new uint8_t[_buffer_size];
		_start_time = av_gettime_relative();
		_time_base = { 1,AV_TIME_BASE };
		_pixel_fmt = AV_PIX_FMT_BGRA;
		error = init_duplication();
		if (error != ERROR_NO)  break;
		_inited = true;
	} while (0);
	if (error != ERROR_NO) {
		printf("%s,last error:%s", err2str(error), system_error::error2str(GetLastError()).c_str());
	}

	return error;
}

int Record_Desktop_DXGI::start()
{
	if (_running == true) {
		printf("record desktop duplication is already running");
		return ERROR_NO;
	}
	if (_inited == false) return NORMAL_NEED_INIT;
	_running = true;
	_thread = std::thread(std::bind(&Record_Desktop_DXGI::record_func, this));
	return ERROR_NO;
}

int Record_Desktop_DXGI::stop()
{
	_running = false;
	if (_thread.joinable()) _thread.join();
	return ERROR_NO;
}

void Record_Desktop_DXGI::clean_up()
{
	_inited = false;
	if (_buffer) delete[] _buffer;
	_buffer = nullptr;
	if (_cursor_info.buff) {
		delete[] _cursor_info.buff;
		_cursor_info.buff = nullptr;
	}
	ZeroMemory(&_cursor_info, sizeof(_cursor_info));
	//Clean up duplication interfaces
	clean_duplication();
	//Clean up d3d11 interfaces
	clean_d3d11();
	//finally free d3d11 & dxgi library
    if (_d3d11) System_Helper::free_system_library(_d3d11);
	_d3d11 = nullptr;
    if (_dxgi) System_Helper::free_system_library(_dxgi);
	_dxgi = nullptr;
}

int Record_Desktop_DXGI::get_dst_adapter(IDXGIAdapter** adapter)
{
	int error = 0;
	do {
		auto adapters = D3D_Helper::get_adapters(&error, true);
		if (error != ERROR_NO || adapters.size() == 0) break;
		for (std::list<IDXGIAdapter*>::iterator itr = adapters.begin(); itr != adapters.end(); itr++) {
			IDXGIOutput* adapter_output = nullptr;
			DXGI_ADAPTER_DESC adapter_desc = { 0 };
			DXGI_OUTPUT_DESC adapter_output_desc = { 0 };
			(*itr)->GetDesc(&adapter_desc);
			printf("adaptor:%s", utils_string::unicode_ascii(adapter_desc.Description).c_str());
			unsigned int n = 0;
			RECT output_rect;
			while ((*itr)->EnumOutputs(n, &adapter_output) != DXGI_ERROR_NOT_FOUND)
			{
				HRESULT hr = adapter_output->GetDesc(&adapter_output_desc);
				if (FAILED(hr)) continue;
				output_rect = adapter_output_desc.DesktopCoordinates;
                printf("output:%s left:%d top:%d right:%d bottom:%d",
                       utils_string::unicode_ascii(adapter_output_desc.DeviceName).c_str(),
                       static_cast<int>(output_rect.left),
                       static_cast<int>(output_rect.top),
                       static_cast<int>(output_rect.right),
                       static_cast<int>(output_rect.bottom));
				if (output_rect.left <= _rect.left &&
					output_rect.top <= _rect.top &&
					output_rect.right >= _rect.right &&
					output_rect.bottom >= _rect.bottom) {
					error = ERROR_NO;
					break;
				}
				++n;
			}
			if (error != R_DXGI_FOUND_ADAPTER_FAILED) {
				*adapter = *itr;
				break;
			}
		}

	} while (0);
	return error;
}

int Record_Desktop_DXGI::create_d3d_device(IDXGIAdapter* adapter, ID3D11Device** device)
{
	int error = ERROR_NO;
	do {
		PFN_D3D11_CREATE_DEVICE create_device =
			(PFN_D3D11_CREATE_DEVICE)GetProcAddress(_d3d11, "D3D11CreateDevice");
		if (!create_device) {
			error = R_D3D_GET_PROC_FAILED;
			break;
		}

		HRESULT hr = S_OK;

		// Driver types supported
		// If you set the pAdapter parameter to a non - NULL value,
		// you must also set the DriverType parameter to the D3D_DRIVER_TYPE_UNKNOWN value.
		D3D_DRIVER_TYPE driver_types[] =
		{
			D3D_DRIVER_TYPE_UNKNOWN,
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT n_driver_types = ARRAYSIZE(driver_types);

		// Feature levels supported
		D3D_FEATURE_LEVEL feature_levels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_1
		};
		UINT n_feature_levels = ARRAYSIZE(feature_levels);
		D3D_FEATURE_LEVEL feature_level;
		// Create device
		for (UINT driver_index = 0; driver_index < n_driver_types; ++driver_index)
		{
			hr = create_device(adapter, driver_types[driver_index], nullptr, 0, feature_levels, n_feature_levels,
				D3D11_SDK_VERSION, device, &feature_level, &_d3d_ctx);
			if (SUCCEEDED(hr)) break;
		}
		if (FAILED(hr))
		{
			error = R_D3D_CREATE_DEVICE_FAILED;
			break;
		}
	} while (0);
	return error;
}

int Record_Desktop_DXGI::init_d3d11()
{
	int error = ERROR_NO;
	do {
		IDXGIAdapter* adapter = nullptr;
		error = get_dst_adapter(&adapter);
		if (error != ERROR_NO) break;
		error = create_d3d_device(adapter, &_d3d_device);
		if (error != ERROR_NO) break;
		//No need for grab full screen,but in move & dirty rects copy
	} while (0);
	return error;
}

void Record_Desktop_DXGI::clean_d3d11()
{
	if (_d3d_device) _d3d_device->Release();
	_d3d_device = nullptr;
	if (_d3d_ctx) _d3d_ctx->Release();
	_d3d_ctx = nullptr;
	if (_d3d_vshader) _d3d_vshader->Release();
	_d3d_vshader = nullptr;
	if (_d3d_pshader) _d3d_pshader->Release();
	_d3d_pshader = nullptr;
	if (_d3d_inlayout) _d3d_inlayout->Release();
	_d3d_inlayout = nullptr;
	if (_d3d_samplerlinear) _d3d_samplerlinear->Release();
	_d3d_samplerlinear = nullptr;
}

int Record_Desktop_DXGI::init_duplication()
{
	int error = ERROR_NO;
	do {
		// Get DXGI device
		IDXGIDevice* dxgi_device = nullptr;
		HRESULT hr = _d3d_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));
		if (FAILED(hr))
		{
			error = R_D3D_QUERYINTERFACE_FAILED;
			break;
		}

		// Get DXGI adapter
		IDXGIAdapter* dxgi_adapter = nullptr;
		hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));
		dxgi_device->Release();
		dxgi_device = nullptr;
		if (FAILED(hr))
		{
			error = R_DUP_GET_PARENT_FAILED;
			break;
		}
		// Get output
		IDXGIOutput* dxgi_output = nullptr;
		hr = dxgi_adapter->EnumOutputs(_output_index, &dxgi_output);
		dxgi_adapter->Release();
		dxgi_adapter = nullptr;
		if (FAILED(hr))
		{
			error = R_DUP_ENUM_OUTPUT_FAILED;
			break;
		}
		dxgi_output->GetDesc(&_output_des);
		// QI for Output 1
		IDXGIOutput1* dxgi_output1 = nullptr;
		hr = dxgi_output->QueryInterface(__uuidof(dxgi_output1), reinterpret_cast<void**>(&dxgi_output1));
		dxgi_output->Release();
		dxgi_output = nullptr;
		if (FAILED(hr))
		{
			error = R_DUP_QI_FAILED;
			break;
		}

		// Create desktop duplication
		hr = dxgi_output1->DuplicateOutput(_d3d_device, &_duplication);
		dxgi_output1->Release();
		dxgi_output1 = nullptr;
		if (FAILED(hr))
		{
			error = R_DUP_DUPLICATE_FAILED;
			if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
			{
				error = R_DUP_DUPLICATE_MAX_FAILED;
			}
			printf("duplicate output failed,%lld", static_cast<long long>(hr));
			break;
		}
	} while (0);
	return error;
}

int Record_Desktop_DXGI::free_duplicated_frame()
{
	HRESULT hr = _duplication->ReleaseFrame();
	if (FAILED(hr))
	{
		return R_DUP_RELEASE_FRAME_FAILED;
	}
	if (_image)
	{
		_image->Release();
		_image = nullptr;
	}
	return R_DUP_RELEASE_FRAME_FAILED;
}

void Record_Desktop_DXGI::clean_duplication()
{
	if (_duplication) _duplication->Release();
	if (_image) _image->Release();
	_duplication = nullptr;
	_image = nullptr;
}

bool Record_Desktop_DXGI::attatch_desktop()
{
	HDESK desktop = nullptr;
	desktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!desktop)
	{
		// We do not have access to the desktop so request a retry
		return false;
	}
	// Attach desktop to this thread
	bool battached = SetThreadDesktop(desktop) != 0;
	CloseDesktop(desktop);
	if (!battached)
	{
		// We do not have access to the desktop so request a retry
		return false;
	}
	return true;
}

int Record_Desktop_DXGI::get_desktop_image(DXGI_OUTDUPL_FRAME_INFO* frame_info)
{
	IDXGIResource* dxgi_res = nullptr;
	// Get new frame
	HRESULT hr = _duplication->AcquireNextFrame(500, frame_info, &dxgi_res);
	// Timeout will return when desktop has no chane
	if (hr == DXGI_ERROR_WAIT_TIMEOUT) return NORMAL_TIMEOUT;
	if (FAILED(hr)) return R_DUP_ACQUIRE_FRAME_FAILED;

	// QI for IDXGIResource
	hr = dxgi_res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&_image));
	dxgi_res->Release();
	dxgi_res = nullptr;
	if (FAILED(hr)) return R_DUP_QI_FRAME_FAILED;

	// Copy old description
	D3D11_TEXTURE2D_DESC frame_desc;
	_image->GetDesc(&frame_desc);

	// Create a new staging buffer for fill frame image
	ID3D11Texture2D* new_image = NULL;
	frame_desc.Usage = D3D11_USAGE_STAGING;
	frame_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	frame_desc.BindFlags = 0;
	frame_desc.MiscFlags = 0;
	frame_desc.MipLevels = 1;
	frame_desc.ArraySize = 1;
	frame_desc.SampleDesc.Count = 1;
	frame_desc.SampleDesc.Quality = 0;
	hr = _d3d_device->CreateTexture2D(&frame_desc, NULL, &new_image);
	if (FAILED(hr)) return R_DUP_CREATE_TEXTURE_FAILED;
	// Copy next staging buffer to new staging buffer
	_d3d_ctx->CopyResource(new_image, _image);
	// Should calc the row pitch ,and compare dst row pitch with frame row pitch
	// Create staging buffer for map bits
	IDXGISurface* dxgi_surface = NULL;
	hr = new_image->QueryInterface(__uuidof(IDXGISurface), (void**)(&dxgi_surface));
	new_image->Release();
	if (FAILED(hr)) return R_DUP_QI_DXGI_FAILED;

	// Map buff to mapped rect structure
	DXGI_MAPPED_RECT mapped_rect;
	hr = dxgi_surface->Map(&mapped_rect, DXGI_MAP_READ);
	if (FAILED(hr)) return R_DUP_MAP_FAILED;
	int dst_offset_x = _rect.left - _output_des.DesktopCoordinates.left;
	int dst_offset_y = _rect.top - _output_des.DesktopCoordinates.top;
	int dst_rowpitch = min(frame_desc.Width, _rect.right - _rect.left) * 4;
	int dst_colpitch = min(_height, _output_des.DesktopCoordinates.bottom - _output_des.DesktopCoordinates.top - dst_offset_y);
	for (int h = 0; h < dst_colpitch; h++) {
		memcpy_s(_buffer + h * dst_rowpitch, dst_rowpitch,
			(BYTE*)mapped_rect.pBits + (h + dst_offset_y) * mapped_rect.Pitch + dst_offset_x * 4, min(mapped_rect.Pitch, dst_rowpitch));
	}
	dxgi_surface->Unmap();
	dxgi_surface->Release();
	dxgi_surface = nullptr;
	return ERROR_NO;
}

int Record_Desktop_DXGI::get_desktop_cursor(const DXGI_OUTDUPL_FRAME_INFO* frame_info)
{
	// A non-zero mouse update timestamp indicates that there is a mouse position update and optionally a shape change
	if (frame_info->LastMouseUpdateTime.QuadPart == 0) return ERROR_NO;
	bool b_updated = true;
	// Make sure we don't update pointer position wrongly
	// If pointer is invisible, make sure we did not get an update from another output that the last time that said pointer
	// was visible, if so, don't set it to invisible or update.
	if (!frame_info->PointerPosition.Visible && (_cursor_info.output_index != _output_index)) b_updated = false;
	// If two outputs both say they have a visible, only update if new update has newer timestamp
	if (frame_info->PointerPosition.Visible && _cursor_info.visible && (_cursor_info.output_index != _output_index) && (_cursor_info.pre_timestamp.QuadPart > frame_info->LastMouseUpdateTime.QuadPart)) b_updated = false;
	// Update position
	if (b_updated)
	{
		_cursor_info.position.x = frame_info->PointerPosition.Position.x + _output_des.DesktopCoordinates.left;
		_cursor_info.position.y = frame_info->PointerPosition.Position.y + _output_des.DesktopCoordinates.top;
		_cursor_info.output_index = _output_index;
		_cursor_info.pre_timestamp = frame_info->LastMouseUpdateTime;
		_cursor_info.visible = frame_info->PointerPosition.Visible != 0;
	}

	// No new shape only update cursor positions & visible state
	if (frame_info->PointerShapeBufferSize == 0)
	{
		return ERROR_NO;
	}

	// Old buffer too small
	if (frame_info->PointerShapeBufferSize > _cursor_info.size)
	{
		if (_cursor_info.buff)
		{
			delete[] _cursor_info.buff;
			_cursor_info.buff = nullptr;
		}
		_cursor_info.buff = new (std::nothrow) BYTE[frame_info->PointerShapeBufferSize];
		if (!_cursor_info.buff)
		{
			_cursor_info.size = 0;
			return NORMAL_ALLOCATE_FAILED;
		}

		// Update buffer size
		_cursor_info.size = frame_info->PointerShapeBufferSize;
	}

	// Get shape
	UINT BufferSizeRequired;
	HRESULT hr = _duplication->GetFramePointerShape(frame_info->PointerShapeBufferSize, reinterpret_cast<VOID*>(_cursor_info.buff), &BufferSizeRequired, &(_cursor_info.shape));
	if (FAILED(hr))
	{
		delete[] _cursor_info.buff;
		_cursor_info.buff = nullptr;
		_cursor_info.size = 0;
		return R_DUP_GET_CURSORSHAPE_FAILED;
	}

	return ERROR_NO;
}

static unsigned int bit_reverse(unsigned int n)
{

	n = ((n >> 1) & 0x55555555) | ((n << 1) & 0xaaaaaaaa);
	n = ((n >> 2) & 0x33333333) | ((n << 2) & 0xcccccccc);
	n = ((n >> 4) & 0x0f0f0f0f) | ((n << 4) & 0xf0f0f0f0);
	n = ((n >> 8) & 0x00ff00ff) | ((n << 8) & 0xff00ff00);
	n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000);
	return n;
}

void Record_Desktop_DXGI::draw_cursor()
{
	if (_cursor_info.visible == false) return;
	int cursor_width = 0, cursor_height = 0, left = 0, top = 0;
	cursor_width = _cursor_info.shape.Width;
	cursor_height = _cursor_info.shape.Height;
	// In case that,the value of position is negative value
	left = abs(_cursor_info.position.x - _rect.left);
	top = abs(_cursor_info.position.y - _rect.top);
	// Notice here
	if (_cursor_info.shape.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME) cursor_height = cursor_height / 2;

	//Skip invisible pixel
	cursor_width = min(_width - left, cursor_width);
	cursor_height = min(_height - top, cursor_height);

	//printf("left:%d top:%d width:%d height:%d type:%d", left, top, cursor_width, height, _cursor_info.shape.Type);

	switch (_cursor_info.shape.Type)
	{

		// The pointer type is a color mouse pointer,
		// which is a color bitmap. The bitmap's size
		// is specified by width and height in a 32 bpp
		// ARGB DIB format.
		// should trans cursor to BGRA?
	case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
	{
		unsigned int* cursor_32 = reinterpret_cast<unsigned int*>(_cursor_info.buff);
		unsigned int* screen_32 = reinterpret_cast<unsigned int*>(_buffer);
		for (int row = 0; row < cursor_height; row++) {
			for (int col = 0; col < cursor_width; col++) {
				unsigned int cur_cursor_val = cursor_32[col + (row * (_cursor_info.shape.Pitch / sizeof(UINT)))];
				//Skip black or empty value
				if (cur_cursor_val == 0x00000000) continue;
				else screen_32[(abs(top) + row) * _width + abs(left) + col] = cur_cursor_val;//bit_reverse(cur_cursor_val);
			}
		}
		break;
	}

	// The pointer type is a monochrome mouse pointer,
	// which is a monochrome bitmap. The bitmap's size
	// is specified by width and height in a 1 bits per
	// pixel (bpp) device independent bitmap (DIB) format
	// AND mask that is followed by another 1 bpp DIB format
	// XOR mask of the same size.
	case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
	{
		unsigned int* cursor_32 = reinterpret_cast<unsigned int*>(_cursor_info.buff);
		unsigned int* screen_32 = reinterpret_cast<unsigned int*>(_buffer);

		for (int row = 0; row < cursor_height; row++) {
			BYTE MASK = 0x80;
			for (int col = 0; col < cursor_width; col++) {
				// Get masks using appropriate offsets
				BYTE AndMask = _cursor_info.buff[(col / 8) + (row * (_cursor_info.shape.Pitch))] & MASK;
				BYTE XorMask = _cursor_info.buff[(col / 8) + ((row + cursor_height) * (_cursor_info.shape.Pitch))] & MASK;
				UINT AndMask32 = (AndMask) ? 0xFFFFFFFF : 0xFF000000;
				UINT XorMask32 = (XorMask) ? 0x00FFFFFF : 0x00000000;

				// Set new pixel
				screen_32[(abs(top) + row) * _width + abs(left) + col] = (screen_32[(abs(top) + row) * _width + abs(left) + col] & AndMask32) ^ XorMask32;

				// Adjust mask
				if (MASK == 0x01) MASK = 0x80;
				else MASK = MASK >> 1;

			}
		}
		break;
	}
	// The pointer type is a masked color mouse pointer.
	// A masked color mouse pointer is a 32 bpp ARGB format
	// bitmap with the mask value in the alpha bits. The only
	// allowed mask values are 0 and 0xFF. When the mask value
	// is 0, the RGB value should replace the screen pixel.
	// When the mask value is 0xFF, an XOR operation is performed
	// on the RGB value and the screen pixel; the result replaces the screen pixel.
	case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
	{
		unsigned int* cursor_32 = reinterpret_cast<unsigned int*>(_cursor_info.buff);
		unsigned int* screen_32 = reinterpret_cast<unsigned int*>(_buffer);
		for (int row = 0; row < cursor_height; row++) {
			for (int col = 0; col < cursor_width; col++) {
				unsigned int cur_cursor_val = cursor_32[col + (row * (_cursor_info.shape.Pitch / sizeof(UINT)))];
				unsigned int cur_screen_val = screen_32[(abs(top) + row) * _width + abs(left) + col];
				unsigned int mask_val = 0xFF000000 & cur_cursor_val;
				if (mask_val) {
					//0xFF: XOR operation is performed on the RGB value and the screen pixel
					cur_screen_val = (cur_screen_val ^ cur_cursor_val) | 0xFF000000;
				}
				else {
					//0x00: the RGB value should replace the screen pixel
					cur_screen_val = cur_cursor_val | 0xFF000000;
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

void Record_Desktop_DXGI::do_sleep(int64_t dur, int64_t pre, int64_t now)
{
	int64_t delay = now - pre;
	dur = delay > dur ? max(0, dur - (delay - dur)) : (dur + dur - delay);

	//printf("%lld", delay);

	if (dur)
		av_usleep(dur);
}

void Record_Desktop_DXGI::record_func()
{
	AVFrame* frame = av_frame_alloc();
	int64_t pre_pts = 0, dur = AV_TIME_BASE / _fps;

	int error = ERROR_NO;

	//Should init after desktop attatched
	//error = init_duplication();
	//if (error != ERROR_NO) {
	//	al_fatal("duplication initialize failed %s,last error :%s", err2str(error),
	// system_error::error2str(GetLastError()).c_str());
	//	if (_on_error) _on_error(error);
	//	return;
	//}

	DXGI_OUTDUPL_FRAME_INFO frame_info;
	while (_running)
	{
		//Timeout is no new picture,no need to update
		if ((error = get_desktop_image(&frame_info)) == NORMAL_TIMEOUT) continue;

		if (error != ERROR_NO) {
			while (_running)
			{
				Sleep(300);
				clean_duplication();
				if ((error = init_duplication()) != ERROR_NO) {
					//if (_on_error) _on_error(error);
				}
				else break;
			}
			continue;
		}
		if ((error = get_desktop_cursor(&frame_info)) == ERROR_NO) draw_cursor();

		free_duplicated_frame();
		frame->pts = av_gettime_relative();
		frame->pkt_dts = frame->pts;
		frame->pkt_pts = frame->pts;
		frame->width = _width;
		frame->height = _height;
		frame->format = AV_PIX_FMT_BGRA;
		frame->pict_type = AV_PICTURE_TYPE_NONE;
		frame->pkt_size = _width * _height * 4;
		av_image_fill_arrays(frame->data,
			frame->linesize,
			_buffer,
			AV_PIX_FMT_BGRA,
			_width,
			_height,
			1
		);
		if (_cb_data) _cb_data(_cb_obj, frame);
		do_sleep(dur, pre_pts, frame->pts);
		pre_pts = frame->pts;
	}
	av_frame_free(&frame);
}
