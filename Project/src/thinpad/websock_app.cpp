#include "websock_app.h"
#include "common.h"

void SerialDataHandler::NewBinFrame()
{
    DBG_MSG("new");
    tmp_cnt=0;
}

void SerialDataHandler::NewTextFrame()
{
    DBG_MSG("new");
    tmp_cnt=0;
}

void SerialDataHandler::EndOfFrame()
{
    DBG_MSG("end recved=%d", tmp_cnt);
}

void SerialDataHandler::FrameData(void *payload, int len)
{
    // DBG_MSG("len: %d", len);
    tmp_cnt+=len;
}

void SerialDataHandler::FrameSent()
{
    DBG_MSG("sent");
}
