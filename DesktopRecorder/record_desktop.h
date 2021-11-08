#ifndef RECORD_DESKTOP_H
#define RECORD_DESKTOP_H

extern "C" {
#include<libavcodec/avcodec.h>
}

#include <thread>

class Record_Desktop {
public:
	virtual ~Record_Desktop() = 0;
	virtual void init() = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
	typedef void (*on_cb_data)(void *obj,void* data);
	void regist_cb(void* obj, on_cb_data cb) {
		_obj = obj;
		_cb_data = cb;
	}
protected:
	void clean_up();
	void* _obj;
private:
	on_cb_data _cb_data;

};

#endif //RECORD_DESKTOP_H
