/* *
 * Author:     dengjinhui <dengjinhui@cdzs.com>
 *
 * Maintainer: dengjinhui <dengjinhui@cdzs.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "record_contrl.h"

Record_Contrl::Record_Contrl()
{
}

void Record_Contrl::init()
{
    dxgi_capture = new DXGI_FrameCapturer;
}

void Record_Contrl::start()
{
    _thread = std::thread(std::bind(&Record_Contrl::record_loop, this));
}

void Record_Contrl::setFPS(int32_t fps)
{
    record_fps = fps;
}

void Record_Contrl::record_loop()
{
    CapturedFrame *frame = new CapturedFrame;
    frame->width = dxgi_capture->getWidth();
    frame->height = dxgi_capture->getHeight();
    frame->lenght = dxgi_capture->getLenght();
    frame->buffer = new unsigned char[dxgi_capture->getLenght()];
    frame->isUpdate = true;
    record_fps = record_fps > DefautFPS || record_fps <= 0 ? DefautFPS: record_fps;
    while (_running) {
#ifdef WIN32
        Sleep(1000.0f/record_fps);
        if(!frame->isUpdate){
            continue;
        }
        memset(frame->buffer,0,dxgi_capture->getLenght());
        if (tryCaptureFrame(dxgi_capture,frame->buffer)) {
            frame->isUpdate = false;
            //_on_image_data(_obj, frame);
            //QMetaObject::invokeMethod(_obj,"getFrame",Qt::AutoConnection,Q_ARG(Frame*,frame));
        }
#endif
#ifdef linux
        usleep(1000.0f/record_fps);
        //...TO-DO
#endif
    }
    delete frame;
}

void Record_Contrl::warp_record_cb(void* obj, CapturedFrame* frame)
{
    Record_Contrl* p = reinterpret_cast<Record_Contrl*>(obj);
    if (p) p->on_desktop_data(frame);
}

void Record_Contrl::on_desktop_data(CapturedFrame* data)
{

}


