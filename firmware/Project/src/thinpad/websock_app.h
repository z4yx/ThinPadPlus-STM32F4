#ifndef WEBSOCK_APP_H__
#define WEBSOCK_APP_H__

#include "WebSocketDataHandler.h"
class SerialDataHandler : public WebSocketDataHandler
{
    enum MsgType
    {
        MsgUnknown,
        MsgData = 'D',
        MsgCloseSerial = 'C',
        MsgOpenSerial = 'O',
    };
    uint8_t msg_type, ctl_buf[16], ctl_ptr;
    bool firstByte;
public:
    void NewBinFrame();
    void NewTextFrame();
    void EndOfFrame();
    void FrameData(void *payload, int len);
    void FrameSent();
};

#endif
