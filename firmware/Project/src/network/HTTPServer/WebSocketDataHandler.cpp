#include "WebSocketDataHandler.h"
#include "HTTPWebSocketHandler.h"

void WebSocketDataHandler::destroy(){
    connected=false;
    parent=NULL;
}

void WebSocketDataHandler::enable(HTTPWebSocketStreamingState* _parent){
    connected=true;
    parent=_parent;
}

bool WebSocketDataHandler::SendFrameAsync(void *payload, uint64_t payloadLength, bool text){
    if(parent)
        return parent->SendFrameAsyc(payload, payloadLength, text);
    return false;
}
