#ifndef RECORD_CONTRL_H
#define RECORD_CONTRL_H

#include "record_desktop.h"
#include "record_desktop_dxgi.h"

class Record_Contrl {
public:
	Record_Contrl();
	void init();
	static void warp_cb(void* obj, void *data);
	void on_desktop_data(void* data);
private:
	Record_Desktop* record_desktop;
};

#endif //RECORD_CONTRL_H
