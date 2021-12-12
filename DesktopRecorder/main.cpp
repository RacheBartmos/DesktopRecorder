#include "record_contrl.h"

int main(int argc,char** argv)
{
    Record_Contrl rc;
    rc.init();
    rc.setFPS(1);
    rc.start();
    while (true) {

    }
    return 0;
}
