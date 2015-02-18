#include "HTTPWebSocketHandler.h"
#include "md5.h"
// #include "TemperatureSensor.h"
// #include "RGBLed.h"

// extern TemperatureSensor sensor;
// extern RGBLed rgb;

HTTPWebSocketConnectingState::HTTPWebSocketConnectingState() {
    memset(challange, 0, 16);
}

HTTPStatus HTTPWebSocketConnectingState::init(HTTPConnection *conn) {
    uint32_t key1Part= computeKeyPart(conn->getField("Sec-WebSocket-Key1"));
    uint32_t key2Part= computeKeyPart(conn->getField("Sec-WebSocket-Key2"));

    challange[0]= (key1Part >> 24) & 0xFF;
    challange[1]= (key1Part >> 16) & 0xFF;
    challange[2]= (key1Part >> 8) & 0xFF;
    challange[3]= (key1Part >> 0) & 0xFF;
    challange[4]= (key2Part >> 24) & 0xFF;
    challange[5]= (key2Part >> 16) & 0xFF;
    challange[6]= (key2Part >> 8) & 0xFF;
    challange[7]= (key2Part >> 0) & 0xFF;

    char headers[1024];
    char *str;
    
    memset(headers, 0, 1024);
    strcat(headers, "Upgrade: WebSocket\r\n");
    strcat(headers, "Connection: Upgrade\r\n");

    // The host field string can have junk at the end...
    str= (char*) conn->getField("Host");
    for (char *p= str; *p != 0; p++) {
        if(!isprint(*p)) {
            *p= 0;
        }
    }
    strcat(headers, "Sec-WebSocket-Location: ws://");
    strcat(headers, str);
    strcat(headers, conn->getURL());
    strcat(headers, "\r\n");
    
    // The host field string can have junk at the end...
    str= (char*) conn->getField("Origin");
    for (char *p= str; *p != 0; p++) {
        if(!isprint(*p)) {
            *p= 0;
        }
    }
    strcat(headers, "Sec-WebSocket-Origin: ");
    strcat(headers, str);
    strcat(headers, "\r\n");
    conn->setLength(16);
    conn->setHeaderFields(headers);
    
    printf("\n%s\n", headers);
    
    return HTTP_SwitchProtocols;
}

HTTPHandle HTTPWebSocketConnectingState::data(HTTPConnection *conn, void *data, int len) {
    // The body needs to hold key3, and only key3
    if (len != 8) {
        printf("key3 length error\n");
        return HTTP_Failed;
    }

    // Copy key3 into the challange
    memcpy(challange+8, data, 8);

    printf("Challange: ");
    for (int i= 0; i<16; i++) {
        printf("%02x ", challange[i]);
    }
    printf("\n");

    return HTTP_Success;
}

HTTPHandle HTTPWebSocketConnectingState::send(HTTPConnection *conn, int maxData) {
    MD5 md5;
    md5.update(challange, 16);
    const char *hex= md5.finalize().hexdigest().c_str();
    char binary[16];
    hexStringToBinary(hex, binary);
    conn->write(binary, 16, (TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE));

    printf("MD5 bin: ");
    for (int i= 0; i<16; i++) {
        printf("%02x ", binary[i]);
    }
    printf("\n");

    delete conn->data;
    conn->data= new HTTPWebSocketStreamingState();
    return HTTP_Success;
}

uint32_t HTTPWebSocketConnectingState::extractNumber(const char *keyValue) {
    uint32_t value= 0;
    while (*keyValue) {
        if (isdigit(*keyValue)) {
            value= value*10 + (*keyValue-'0');
        };
        keyValue++;
    };
    return value;
}

uint32_t HTTPWebSocketConnectingState::countSpaces(const char *keyValue) {
    uint32_t count= 0;
    while (*keyValue) {
        if (*keyValue == 0x20) {
            count++;
        }
        keyValue++;
    }
    return count;
}

uint32_t HTTPWebSocketConnectingState::computeKeyPart(const char *keyValue) {
    if (keyValue != NULL) {
        printf("key:%s:\n", keyValue);
        uint32_t keyPart= extractNumber(keyValue) / countSpaces(keyValue);
        printf("keyPart: %08x\n", keyPart);
        return keyPart;
    }
    // No key value
    return 0;
}

void HTTPWebSocketConnectingState::hexStringToBinary(const char *hex, char *binary) {
    while (*hex) {
        char c;

        c= *hex++;
        *binary= (c < '@' ? c-'0' : c-'a'+10) << 4;
        c= *hex++;
        *binary |= (c < '@' ? c-'0' : c-'a'+10);
        binary++;
    }
}

//-----

HTTPWebSocketStreamingState::HTTPWebSocketStreamingState() /*: led(LED2)*/ {
    sendCnt= 0;
}

HTTPStatus HTTPWebSocketStreamingState::init(HTTPConnection *conn) {
    return HTTP_InternalServerError; // ignored
}

HTTPHandle HTTPWebSocketStreamingState::data(HTTPConnection *conn, void *data, int len) {
    char *str= ((char*)data)+1; // skip leading 0x00
    str[len]= 0;    // stop on trailing 0xFF
    //printf("IN:%d:%s\n", len, str);
    // rgb.setRgb(str);
    return HTTP_Success;
}

HTTPHandle HTTPWebSocketStreamingState::send(HTTPConnection *conn, int maxData) {
    char buffer[80];
    int len;
    
    if (sendCnt > 1000) {
        sprintf(buffer, "%c%d,%d,%d\n%c", 0x00, 1, 2, 3, 0xFF);
        len= strlen(buffer+1);
        conn->write(buffer, len+1);
        sendCnt= 0;
        //led= !led;
    } else {
        // sensor.measure();
        sendCnt++;
    }

    return HTTP_Success;
}

//-----

// Register headers required by this handler.  Headers that are not registered will
// be stripped by the server.
void HTTPWebSocketHandler::reg(HTTPServer *server) {
    server->registerField("Sec-WebSocket-Key1");
    server->registerField("Sec-WebSocket-Key2");
    server->registerField("Host");
    server->registerField("Origin");
}

HTTPStatus HTTPWebSocketHandler::init(HTTPConnection *conn) const {
    // Todo: validate headers
    HTTPWebSocketState *state= new HTTPWebSocketConnectingState();
    conn->data= state;
    HTTPStatus status= state->init(conn);
    printf("init: %d\n", status);
    return status;
}

HTTPHandle HTTPWebSocketHandler::data(HTTPConnection *conn, void *data, int len) const {
    HTTPWebSocketState *state= (HTTPWebSocketState *) conn->data;
    HTTPHandle result= state->data(conn, data, len);
    //printf("data: %d\n", result);
    return result;
}

HTTPHandle HTTPWebSocketHandler::send(HTTPConnection *conn, int maxData) const {
    HTTPWebSocketState *state= (HTTPWebSocketState *) conn->data;
    HTTPHandle result= state->send(conn, maxData);
    //printf("send: %d\n", result);
    return result;
}

