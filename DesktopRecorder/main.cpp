#include <QGuiApplication>
#include "h264_encoder.h"

int main(int argc,char** argv)
{
    QGuiApplication a(argc, argv);

    h264_encoder encoder;
    encoder.png2mp4("E:/item/video/capture_test_video.mp4");
    exit(0);
    return a.exec();
}
