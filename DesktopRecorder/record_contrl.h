#ifndef RECORD_CONTRL_H
#define RECORD_CONTRL_H

#include "buffer_queue.h"
#include "record_desktop.h"
#include "record_desktop_dxgi.h"

class Record_Contrl {
public:
	Record_Contrl();
	void init();
    void start();
private:
    static void warp_record_cb(void* obj, AVFrame *data);
    void on_desktop_data(AVFrame* data);
	Record_Desktop* record_desktop;
};

#endif //RECORD_CONTRL_H
