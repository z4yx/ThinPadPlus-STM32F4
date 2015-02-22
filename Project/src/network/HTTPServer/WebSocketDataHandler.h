#ifndef WebSocketDataHandler_H__
#define WebSocketDataHandler_H__


class WebSocketDataHandler
{
public:
    virtual void NewBinFrame() = 0;
    virtual void NewTextFrame() = 0;
    virtual void EndOfFrame() = 0;
    virtual void FrameData(void *payload, int len) = 0;
    virtual void FrameSent() = 0;
    // void SendFrameAsync(void *payload, uint64_t payloadLength);
    // bool IsConnected();
};

#endif
