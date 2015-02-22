#ifndef HTTP_WEB_SOCKET_HANDLER_H
#define HTTP_WEB_SOCKET_HANDLER_H

#include "HTTPServer.h"
#include "WebSocketDataHandler.h"
#include "sha1.h"

using namespace mbed;

typedef WebSocketDataHandler* (*ObtainDataHandlerFunc)(const char* url);

// Abstrct base class for state delegation
class HTTPWebSocketState : public HTTPData {
public:
    virtual HTTPStatus init(HTTPConnection *conn) = 0;
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len) = 0;
    virtual HTTPHandle send(HTTPConnection *conn, int maxData) = 0;
};

// State class for connecting the WebSocket, performing security handshake
class HTTPWebSocketConnectingState : public HTTPWebSocketState {
public:
    HTTPWebSocketConnectingState();
    virtual HTTPStatus init(HTTPConnection *conn);
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len) {};
    virtual HTTPHandle send(HTTPConnection *conn, int maxData) {};
private:
    // Helper functions
    void hexStringToBinary(const char *hex, char *binary);
    
    char keyBuffer[64];
    char acceptKey[32];
    SHA1Context sha;
};

// State class for streaming the WebSocket
class HTTPWebSocketStreamingState : public HTTPWebSocketState {
private:
    enum opcode_t
    {
        OpCodeContinuation = 0,
        OpCodeText = 1,
        OpCodeBin = 2,
        OpCodeClose = 8,
        OpCodePing = 9,
        OpCodePong = 10,
    };
    enum state_t
    {
        ReadOpCode,
        ReadLen8,
        ReadLen16,
        ReadLen64,
        ReadMask,
        ReadPayload,
    };
    uint64_t payloadLength, payloadRecved;
    uint8_t maskingKey[4];
    uint8_t opcode, readState, readLenCnt;
    bool isFIN;
    int payloadStartPos;
public:
    HTTPWebSocketStreamingState(WebSocketDataHandler *appHandler)
        :readState(ReadOpCode),dataHandler(appHandler){}
    virtual HTTPStatus init(HTTPConnection *conn);
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len);
    virtual HTTPHandle send(HTTPConnection *conn, int maxData);
    WebSocketDataHandler* dataHandler;
};

// HTTP handler for resorces that become WebSockets
class HTTPWebSocketHandler : public HTTPHandler {
public:
    HTTPWebSocketHandler(const char *path, ObtainDataHandlerFunc func) 
        : HTTPHandler(path),obtainDataHandlerFunc(func) {
        printf("HTTPWebSocketHandler: %s\n", path);
    }

protected:
    virtual void reg(HTTPServer *);
    virtual HTTPStatus init(HTTPConnection *conn) const;
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len) const; 
    virtual HTTPHandle send(HTTPConnection *, int) const; 
    ObtainDataHandlerFunc obtainDataHandlerFunc;
};

#endif
