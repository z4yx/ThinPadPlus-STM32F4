#ifndef HTTP_REST_HANDLER_H
#define HTTP_REST_HANDLER_H

#include "HTTPServer.h"
#include "string.h"

using namespace mbed;

#define RESP_LEN 1024

class HTTPRestData : public HTTPData {
public:
    HTTPRestData() {
        memset(response, 0, RESP_LEN);
    };
    
    char response[RESP_LEN];
};

// HTTP handler for RESTful requests
class HTTPRestHandler : public HTTPHandler {
public:
    HTTPRestHandler(const char *path) : HTTPHandler(path) {
        printf("HTTPRestHandler: %s\n", path);
        prefixLength= strlen(path);
    }
    
protected:
    int prefixLength;
    HTTPStatus doGet(char *resource, HTTPConnection *conn) const;
    HTTPStatus doPost(char *resource, HTTPConnection *conn) const;
    
    virtual void reg(HTTPServer *server); 
    virtual HTTPStatus init(HTTPConnection *conn) const;
    virtual HTTPHandle data(HTTPConnection *conn, void *data, int len) const; 
    virtual HTTPHandle send(HTTPConnection *conn, int maxData) const;   
};

#endif
