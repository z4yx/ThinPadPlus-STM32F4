#ifndef WEBSOCK_APP_H__
#define WEBSOCK_APP_H__

#include "WebSocketDataHandler.h"
class SerialDataHandler : public WebSocketDataHandler
{
    int tmp_cnt;
public:
    void NewBinFrame();
    void NewTextFrame();
    void EndOfFrame();
    void FrameData(void *payload, int len);
    void FrameSent();
};

#endif
