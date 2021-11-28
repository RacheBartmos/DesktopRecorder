#ifndef RECORD_CONTRL_H
#define RECORD_CONTRL_H

#include "buffer_queue.h"
#include "record_dxgi.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
const int DefautFPS = 10;
struct CapturedFrame
{
    uint32_t height;
    uint32_t width;
    uint32_t lenght;
    uint8_t* buffer;
    bool     isUpdate = false;
    ~CapturedFrame(){
        delete buffer;
    }
};

class Record_Contrl {
public:
	Record_Contrl();
	void init();
    void start();
    void setFPS(int32_t fps);
private:
    void record_loop();
    static void warp_record_cb(void* obj, CapturedFrame *data);
    void on_desktop_data(CapturedFrame* data);
    DXGI_FrameCapturer *dxgi_capture;
    int record_fps;
    bool _running = true;
    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond_var;
};

#endif //RECORD_CONTRL_H
