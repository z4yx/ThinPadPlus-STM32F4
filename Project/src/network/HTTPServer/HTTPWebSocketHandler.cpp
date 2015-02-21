#include "HTTPWebSocketHandler.h"
#include "base64.h"
// #include "TemperatureSensor.h"
// #include "RGBLed.h"

// extern TemperatureSensor sensor;
// extern RGBLed rgb;

HTTPWebSocketConnectingState::HTTPWebSocketConnectingState() {
}

HTTPStatus HTTPWebSocketConnectingState::init(HTTPConnection *conn) {
    strncpy(keyBuffer, conn->getField("Sec-WebSocket-Key"), sizeof(keyBuffer)-37);
    keyBuffer[sizeof(keyBuffer)-37] = '\0';
    strcat(keyBuffer, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    SHA1Reset( &sha );
    SHA1Input( &sha, (uint8_t*)keyBuffer, strlen(keyBuffer) );
    SHA1Result( &sha );

    for (int i = 0, k = 0; i < 5; ++i)
    {
        unsigned int dw = sha.Message_Digest[i];
        for (int j = 0; j < 4; ++j)
        {
            keyBuffer[k++] = (dw & 0xff000000) >> 24;
            dw <<= 8;
        }
    }

    memset(acceptKey, 0, sizeof(acceptKey));
    base64_encode((uint8_t*)keyBuffer, 20, acceptKey, sizeof(acceptKey));

    char* headers = new char[128];
    headers[0] = '\0';
    strcat(headers, "Upgrade: websocket\r\n");
    strcat(headers, "Connection: Upgrade\r\n");
    strcat(headers, "Sec-WebSocket-Accept: ");
    strcat(headers, acceptKey);
    strcat(headers, "\r\n");
    
    // char *str;
    // // The host field string can have junk at the end...
    // str= (char*) conn->getField("Origin");
    // for (char *p= str; *p != 0; p++) {
    //     if(!isprint(*p)) {
    //         *p= 0;
    //     }
    // }
    // strcat(headers, "Sec-WebSocket-Origin: ");
    // strcat(headers, str);
    // strcat(headers, "\r\n");
    conn->setLength(0);
    conn->setHeaderFields(headers);
    
    printf("\n%s\n", headers);
    
    return HTTP_SwitchProtocols;
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

HTTPStatus HTTPWebSocketStreamingState::init(HTTPConnection *conn) {
    return HTTP_InternalServerError; // ignored
}

HTTPHandle HTTPWebSocketStreamingState::data(HTTPConnection *conn, void *data, int len) {
    uint8_t *ptr = (uint8_t*) data;

    if(len == 0)
        return HTTP_Success;

    // printf("\r\npacket len %d\r\n", len);
    for (int i = 0; i < len; ++i)
    {
        switch(readState){
        case ReadOpCode:
            if(!(ptr[i] & 0x80)){
                printf("%s\r\n", "FIN=0 is not supported yet!");
            }
            opcode = ptr[i] & 0x0f;
            printf("got frame with opcode=%d\r\n", opcode);
            switch(opcode){
            case OpCodeText:
            case OpCodeBin:
            case OpCodePing:
            case OpCodePong:
                break;
            case OpCodeClose:
                printf("%s\r\n", "Close frame received");
                goto EndConnection;
            default:
                printf("Unknown opcode %d\n", opcode);
                goto EndConnection;
            }
            readState = ReadLen8;
            break;
        case ReadLen8:
            if(!(ptr[i] & 0x80)){
                printf("%s\r\n", "Not masked");
                goto EndConnection;
            }
            payloadLength = ptr[i] & 0x7f;
            if(payloadLength == 126){
                readLenCnt = 0;
                readState = ReadLen16;
                payloadLength = 0;
            }else if(payloadLength == 127){
                readLenCnt = 0;
                readState = ReadLen64;
                payloadLength = 0;
            }else{
                readLenCnt = 0;
                readState = ReadMask;
            }
            break;
        case ReadLen16:
            payloadLength = (payloadLength<<8)|ptr[i];
            if(++readLenCnt == 2){
                readLenCnt = 0;
                readState = ReadMask;
            }
            break;
        case ReadLen64:
            payloadLength = (payloadLength<<8)|ptr[i];
            if(++readLenCnt == 8){
                readLenCnt = 0;
                readState = ReadMask;
            }
            break;
        case ReadMask:
            maskingKey[readLenCnt++] = ptr[i];
            if(readLenCnt == 4){
                payloadRecved = 0;
                printf("Receiving %lu bytes data with mask %x,%x,%x,%x\r\n",
                    (uint32_t)payloadLength, maskingKey[0], maskingKey[1], 
                    maskingKey[2], maskingKey[3]);
                if(payloadLength == 0) //an empty frame
                    readState = ReadOpCode; //receiving next frame
                else
                    readState = ReadPayload;
            }
            break;
        case ReadPayload:
            // printf("%c", ptr[i] ^ maskingKey[payloadRecved&3]);
            payloadRecved++;
            if(payloadRecved == payloadLength){
                // printf("\r\n");
                readState = ReadOpCode;//prepare for next frame
            }
            break;
        }
    }
    return HTTP_Success;
EndConnection:
    printf("%s\r\n", "Closing connection");
    return HTTP_SuccessEnded;
}

HTTPHandle HTTPWebSocketStreamingState::send(HTTPConnection *conn, int maxData) {
    // char buffer[80];
    // int len;
    
    // if (sendCnt > 1000) {
    //     sprintf(buffer, "%c%d,%d,%d\n%c", 0x00, 1, 2, 3, 0xFF);
    //     len= strlen(buffer+1);
    //     conn->write(buffer, len+1);
    //     sendCnt= 0;
    //     //led= !led;
    // } else {
    //     // sensor.measure();
    //     sendCnt++;
    // }

    return HTTP_Success;
}

//-----

// Register headers required by this handler.  Headers that are not registered will
// be stripped by the server.
void HTTPWebSocketHandler::reg(HTTPServer *server) {
    server->registerField("Sec-WebSocket-Key");
    server->registerField("Host");
    server->registerField("Origin");
}

HTTPStatus HTTPWebSocketHandler::init(HTTPConnection *conn) const {
    // Todo: validate headers
    HTTPWebSocketState *state= new HTTPWebSocketConnectingState();
    HTTPStatus status= state->init(conn);
    delete state;
    conn->data = new HTTPWebSocketStreamingState();
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

