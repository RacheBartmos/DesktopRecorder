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
	record_desktop = new Record_Desktop_DXGI;
    RECORD_DESKTOP_RECT rect;
    rect = {0,0,2560,1080};
    record_desktop->init(rect,10);
    record_desktop->regist_cb(this,warp_record_cb);
}

void Record_Contrl::start()
{
    if(record_desktop) record_desktop->start();
}

void Record_Contrl::warp_record_cb(void* obj, AVFrame* frame)
{
	Record_Contrl* p = reinterpret_cast<Record_Contrl*>(obj);
    if (p) p->on_desktop_data(frame);
}

void Record_Contrl::on_desktop_data(AVFrame* data)
{

}


