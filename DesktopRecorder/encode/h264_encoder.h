#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#include "headers_ffmpeg.h"
#include "dxcutscreen.h"
#include <QString>

class h264_encoder 
{
public:
    h264_encoder();
    void png2mp4(const QString &fileName);
    void setStatus(bool status);
private:
    DxCutScreen dxcutscreen;
    bool isDxCutReady;
    bool keepCapture;
};

#endif // H264_ENCODER_H
