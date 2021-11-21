#include "video_encoder_nvenc.h"

Video_Encoder_NVENC::Video_Encoder_NVENC()
{
    av_register_all();
    _encoder = NULL;
    _encoder_ctx = NULL;
    _frame = NULL;
    _buff = NULL;
    _buff_size = 0;
    _y_size = 0;
}


Video_Encoder_NVENC::~Video_Encoder_NVENC()
{
    stop();
    cleanup();
}

int Video_Encoder_NVENC::init(int pic_width, int pic_height, int frame_rate, int bit_rate, int qb, int key_pic_sec)
{
    if (_inited == true)
        return ERROR_NO;
    int err = ERROR_NO;
    int ret = 0;

    do {
        _encoder = avcodec_find_encoder_by_name("h264_nvenc");
        if(!_encoder) _encoder = avcodec_find_encoder_by_name("nvenc_h264");

        if (!_encoder) {
            err = ENCODER_FFMPEG_FIND_ENCODER_FAILED;
            break;
        }
        _encoder_ctx = avcodec_alloc_context3(_encoder);
        if (!_encoder_ctx) {
            err = ENCODER_FFMPEG_ALLOC_CONTEXT_FAILED;
            break;
        }

        // ffmpeg -h encoder=h264_nvenc show all encoder options

        const char *rate_control = "cbr"; // cbr | cqp | vbr | lossless
        const char *profile = "baseline"; // baseline | main | high |high444p
        const char *preset = "default";      // default | slow | medium | fast |
                                          // hp | hq | bd | 11 | 11hq | 11hp | lossless | losslesshp

        _encoder_ctx->bit_rate = bit_rate;
        //qb is 0 ~ 100
        qb = max(min(qb, 100), 0);
        //for qmax more larger,quality is more less, max qmax is qmin + 30*(100 - 0)/100 = qmin + 30
        _encoder_ctx->qmin = 30;
        _encoder_ctx->qmax = _encoder_ctx->qmin + 15 * (100 - qb) / 100;

        av_opt_set(_encoder_ctx->priv_data, "profile", profile, 0);
        av_opt_set(_encoder_ctx->priv_data, "preset", preset, 0);
        av_opt_set(_encoder_ctx->priv_data, "level", "auto", 0);
        av_opt_set_int(_encoder_ctx->priv_data, "2pass", false, 0);
        av_opt_set_int(_encoder_ctx->priv_data, "gpu", 0, 0);

        _encoder_ctx->width = pic_width;
        _encoder_ctx->height = pic_height;
        _encoder_ctx->time_base.num = 1;
        _encoder_ctx->time_base.den = frame_rate;
        _encoder_ctx->framerate = { frame_rate,1 };
        _encoder_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

        if (key_pic_sec == 0) _encoder_ctx->gop_size = 250;
        else _encoder_ctx->gop_size = key_pic_sec * _encoder_ctx->time_base.den / _encoder_ctx->time_base.num;
        _encoder_ctx->max_b_frames = 0;//NO B Frame
        _encoder_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        ret = avcodec_open2(_encoder_ctx, _encoder, nullptr);
        if (ret != 0) {
            err = ENCODER_FFMPEG_OPEN_CODEC_FAILED;
            break;
        }
        _frame = av_frame_alloc();
        _buff_size = av_image_get_buffer_size(_encoder_ctx->pix_fmt, _encoder_ctx->width, _encoder_ctx->height, 1);
        _buff = (uint8_t*)av_malloc(_buff_size);
        if (!_buff) {
            break;
        }
        av_image_fill_arrays(_frame->data, _frame->linesize, _buff, _encoder_ctx->pix_fmt, _encoder_ctx->width, _encoder_ctx->height, 1);
        _frame->format = _encoder_ctx->pix_fmt;
        _frame->width = _encoder_ctx->width;
        _frame->height = _encoder_ctx->height;
        _y_size = _encoder_ctx->width * _encoder_ctx->height;
        _time_base = _encoder_ctx->time_base;
        _inited = true;
    } while (0);
    if (err != ERROR_NO) {
        printf("%s,error:%d %lu", err2str(err), ret, GetLastError());
        cleanup();
    }
    return err;
}

int Video_Encoder_NVENC::get_extradata_size()
{
    return _encoder_ctx->extradata_size;
}

const uint8_t * Video_Encoder_NVENC::get_extradata()
{
    return (const uint8_t*)_encoder_ctx->extradata;
}

AVCodecID Video_Encoder_NVENC::get_codec_id()
{
    if (_inited == false) return AV_CODEC_ID_NONE;
    return _encoder->id;
}

void Video_Encoder_NVENC::cleanup()
{
    if (_frame) av_free(_frame);
    _frame = NULL;
    if (_buff) av_free(_buff);
    _buff = NULL;
    if (_encoder) avcodec_close(_encoder_ctx);
    _encoder = NULL;
    if (_encoder_ctx) avcodec_free_context(&_encoder_ctx);
    _encoder_ctx = NULL;
}

void Video_Encoder_NVENC::encode_loop()
{
    AVPacket *packet = av_packet_alloc();
    AVFrame yuv_frame;
    int error = ERROR_NO;
    while (_running)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_cond_notify && _running) _cond_var.wait_for(lock, std::chrono::milliseconds(300));
        while (_buffer_queue->get(_buff, _buff_size, yuv_frame)) {
            _frame->pkt_dts = yuv_frame.pkt_dts;
            _frame->pkt_dts = yuv_frame.pkt_dts;
            _frame->pts = yuv_frame.pts;
            if ((error = encode(_frame, packet)) != ERROR_NO) {
//                if (_on_error)
//                    _on_error(error);
                printf("encode 264 packet failed:%d", error);
                break;
            }
        }
        _cond_notify = false;
    }
    //flush frame in encoder
    encode(NULL, packet);
    av_packet_free(&packet);
}

int Video_Encoder_NVENC::encode(AVFrame * frame, AVPacket * packet)
{
    int ret = avcodec_send_frame(_encoder_ctx, frame);
    if (ret < 0) {
        return ENCODER_FFMPEG_ENCODE_FRAME_FAILED;
    }
    while (ret >= 0) {
        av_init_packet(packet);
        ret = avcodec_receive_packet(_encoder_ctx, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            return ENCODER_FFMPEG_READ_PACKET_FAILED;
        }
        if (ret == 0 && _on_data) _on_data(packet);
        av_packet_unref(packet);
    }
    return ERROR_NO;
}
