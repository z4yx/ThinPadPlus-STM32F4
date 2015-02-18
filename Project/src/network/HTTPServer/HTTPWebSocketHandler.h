#ifndef HTTP_WEB_SOCKET_HANDLER_H
#define HTTP_WEB_SOCKET_HANDLER_H

#include "HTTPServer.h"
// #include "TemperatureSensor.h"
// #include "RGBLed.h"

using namespace mbed;

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
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len);
    virtual HTTPHandle send(HTTPConnection *conn, int maxData);
private:
    // Helper functions
    uint32_t extractNumber(const char *keyValue);
    uint32_t countSpaces(const char *keyValue);
    uint32_t computeKeyPart(const char *keyValue);
    void hexStringToBinary(const char *hex, char *binary);
    
    uint8_t challange[16];
};

// State class for streaming the WebSocket
class HTTPWebSocketStreamingState : public HTTPWebSocketState {
public:
    HTTPWebSocketStreamingState();
    virtual HTTPStatus init(HTTPConnection *conn);
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len);
    virtual HTTPHandle send(HTTPConnection *conn, int maxData);
private:
    int sendCnt;
};

// HTTP handler for resorces that become WebSockets
class HTTPWebSocketHandler : public HTTPHandler {
public:
    HTTPWebSocketHandler(const char *path) : HTTPHandler(path) {
        printf("HTTPWebSocketHandler: %s\n", path);
    }
    
protected:
    virtual void reg(HTTPServer *);
    virtual HTTPStatus init(HTTPConnection *conn) const;
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len) const; 
    virtual HTTPHandle send(HTTPConnection *, int) const;   
};

#endif
