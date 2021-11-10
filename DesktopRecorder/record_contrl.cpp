#include "record_contrl.h"

Record_Contrl::Record_Contrl()
{
}

void Record_Contrl::init()
{
	record_desktop = new Record_Desktop_DXGI;
    record_desktop->regist_cb(this,warp_cb);
}

void Record_Contrl::start()
{
    if(record_desktop) record_desktop->start();
}

void Record_Contrl::warp_cb(void* obj, void* data)
{
	Record_Contrl* p = reinterpret_cast<Record_Contrl*>(obj);
	if (p) p->on_desktop_data(data);
}

void Record_Contrl::on_desktop_data(void* data)
{
}


