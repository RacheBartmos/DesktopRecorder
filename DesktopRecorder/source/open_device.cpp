#include "open_device.h"
#include <QFile>
#include <QDebug>
static bool rec_status = false;

static void set_status(bool _s){rec_status = _s;}

static AVFormatContext *open_device()
{
    int ret = 0;
    char errors[1024] = {0, };

    //ctx
    AVFormatContext *fmt_ctx = nullptr;
    AVDictionary *options = nullptr;

    //[[video device]:[audio device]]
    //0:机器的摄像头
    //1:桌面
    QString devicename = "video=HIK 2K Camera";

    //register audio device
    avdevice_register_all();

    //get format
    AVInputFormat *iformat = av_find_input_format("dshow");

    av_dict_set(&options, "video_size", "640*480", 0);
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "pixel_format","nv12",0);


    //open deivce
    if((ret = avformat_open_input(&fmt_ctx, devicename.toLocal8Bit().data(), iformat, &options)) < 0){
        av_strerror(ret,errors,1024);
        fprintf(stderr,"Failed to open video device, [%d]%s\n",ret,errors);
    }
    return fmt_ctx;
}

//void rec_video(){
//    int ret_ = 0;
//    AVFormatContext *fmt_ctx = nullptr;

//    //pakcet
//    AVPacket pkt;

//    //set log level
//    av_log_set_level(AV_LOG_DEBUG);

//    //start record;

//    set_status(true);

//    //create file
//    QString out = "E:/item/video/test.yuv";

//    //QFile outfile(out);
////    if(!outfile.open(QIODevice::ReadWrite)) {
////        qDebug() << "open file failed";
////        return ;
////    }
//    //QDataStream writeB(&outfile);
//    FILE *outfile = fopen(out.toLocal8Bit().data(),"wb+");

//    fmt_ctx = open_device();
//    while ((ret_ = av_read_frame(fmt_ctx,&pkt)) == 0 && rec_status) {
//        av_log(nullptr,AV_LOG_INFO,"packet size is %d(%p)\n", pkt.size, pkt.data);

//        //encode //size的大小会因为机型、CPU、操作系统而不同 ，这个值应该为（宽*高）*(yuv420=1.5/yuv422=2/yuv444=3)
//        fwrite(pkt.data,1,460800,outfile);
//        fflush(outfile);
//        av_packet_unref(&pkt);
//    }

//}
