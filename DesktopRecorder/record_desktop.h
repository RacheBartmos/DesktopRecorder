#ifndef RECORD_DESKTOP_H
#define RECORD_DESKTOP_H

#include "headers_ffmpeg.h"
#include "defines.h"

#include <atomic>
#include <thread>
#include <functional>

class Record_Desktop {
public:
    virtual ~Record_Desktop() {
    }
    virtual int init(const RECORD_DESKTOP_RECT &rect, const int fps) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    inline const AVRational & get_time_base() { return _time_base; }
    inline int64_t get_start_time() { return _start_time; }
    inline AVPixelFormat get_pixel_fmt() { return _pixel_fmt; }
public:
    inline bool is_recording() { return _running; }
    inline const std::string & get_device_name() { return _device_name; }
    inline const RECORD_DESKTOP_DATA_TYPES get_data_type() { return _data_type; }
    typedef void (*on_cb_data)(void *obj,AVFrame* data);
	void regist_cb(void* obj, on_cb_data cb) {
        _cb_obj = obj;
		_cb_data = cb;
	}
    inline const RECORD_DESKTOP_RECT & get_rect() { return _rect; }
    inline const int get_frame_rate() { return _fps; }
protected:
    virtual void clean_up() = 0;
    void* _cb_obj;
protected:
    std::atomic_bool            _running;
    std::atomic_bool            _paused;
    std::atomic_bool            _inited;
    std::thread                 _thread;
    std::string                 _device_name;
    RECORD_DESKTOP_RECT         _rect;
    RECORD_DESKTOP_DATA_TYPES   _data_type;
    int                         _fps;
    on_cb_data                  _cb_data;
    AVRational                  _time_base;
    int64_t                     _start_time;
    AVPixelFormat               _pixel_fmt;
};

#endif //RECORD_DESKTOP_H
