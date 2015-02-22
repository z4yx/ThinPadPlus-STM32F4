#ifndef WebSocketDataHandler_H__
#define WebSocketDataHandler_H__

#include <cstring>
#include <stdint.h>

class HTTPWebSocketStreamingState;
class WebSocketDataHandler
{
    friend class HTTPWebSocketStreamingState;
    HTTPWebSocketStreamingState* parent;
    bool connected;

    void destroy();
    void enable(HTTPWebSocketStreamingState* _parent);
public:
    WebSocketDataHandler():
        connected(false),parent(NULL){}
    virtual void NewBinFrame() = 0;
    virtual void NewTextFrame() = 0;
    virtual void EndOfFrame() = 0;
    virtual void FrameData(void *payload, int len) = 0;
    virtual void FrameSent() = 0;
    bool SendFrameAsync(void *payload, uint64_t payloadLength);
    bool IsConnected(){return connected;}
};

#endif
