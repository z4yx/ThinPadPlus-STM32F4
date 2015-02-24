#include "websock_app.h"
#include "serial_redirect.h"
#include "common.h"
#include "stdlib.h"

void SerialDataHandler::NewBinFrame()
{
    ERR_MSG("Ignore binary message");
}

void SerialDataHandler::NewTextFrame()
{
    firstByte = true;
    msg_type = MsgUnknown;
}

void SerialDataHandler::EndOfFrame()
{
    int baud;
    switch(msg_type){
        case MsgCloseSerial:
            DBG_MSG("MsgCloseSerial");
            SerialRedirect_Close();
            break;
        case MsgOpenSerial:
            ctl_buf[ctl_ptr] = '\0';
            baud = atoi((char*)ctl_buf);
            DBG_MSG("MsgOpenSerial baud=%d", baud);
            SerialRedirect_Open(baud);
            break;
    }
}

void SerialDataHandler::FrameData(void *_payload, int len)
{
    if(len == 0)
        return;
    uint8_t *payload = (uint8_t*)_payload;
    if(firstByte){
        firstByte = false;
        msg_type = *payload;
        ctl_ptr = 0;
        payload++;
        len--;
    }
    if(msg_type == MsgData){
        SerialRedirect_ToThinpad(payload, len);
    }else if(msg_type == MsgOpenSerial){
        while (len-- && ctl_ptr<sizeof(ctl_buf)-2)
        {
            ctl_buf[ctl_ptr++] = *payload;
            payload++;
        }
    }
    // if(len == 2)
    //     SendFrameAsync((void*)"Good", 4);
}

void SerialDataHandler::FrameSent()
{
    SerialRedirect_FrameSent();
}
