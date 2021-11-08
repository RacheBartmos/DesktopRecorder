#ifndef RECORD_DESKTOP_DXGI_H
#define RECORD_DESKTOP_DXGI_H
#include "record_desktop.h"

class Record_Desktop_DXGI : public Record_Desktop
{
public:
	Record_Desktop_DXGI();
	void init() override;
	void start() override;
	void stop() override;
protected:
	
private:
};

#endif