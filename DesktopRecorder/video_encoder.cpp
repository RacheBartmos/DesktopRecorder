#include "video_encoder.h"

Video_Encoder::Video_Encoder()
{
    _inited = false;
    _running = false;
    _time_base = { 0,AV_TIME_BASE };
    _encoder_id = EID_VIDEO_X264;
    _encoder_type = ET_VIDEO_SOFT;
    _cond_notify = false;
    _buffer_queue = new Buffer_Queue<AVFrame>();
}

Video_Encoder::~Video_Encoder(){
    if(_buffer_queue)
        delete _buffer_queue;
}

int Video_Encoder::start()
{
    int error = ERROR_NO;
    if (_running == true) {
        return error;
    }

    if (_inited == false) {
        return NORMAL_NEED_INIT;
    }
    _running = true;
    _thread = std::thread(std::bind(&Video_Encoder::encode_loop, this));
    return error;
}

void Video_Encoder::stop()
{
    _running = false;
    _cond_notify = true;
    _cond_var.notify_all();
    if (_thread.joinable())
        _thread.join();
}

int Video_Encoder::put(const uint8_t *data, int data_len, AVFrame *frame)
{
    std::unique_lock<std::mutex> lock(_mutex);
    AVFrame frame_cp;
    memcpy(&frame_cp, frame, sizeof(AVFrame));
    _buffer_queue->put(data, data_len, frame_cp);
    _cond_notify = true;
    _cond_var.notify_all();
    return 0;
}

