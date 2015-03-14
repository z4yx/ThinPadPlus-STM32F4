#include "common.h"
#include "HTTPRestHandler.h"
#include "filesystem.h"

#define in_range(c, lo, up)  ((u8_t)c >= lo && (u8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)

HTTPStatus HTTPRestHandler::doGet(char *resource, HTTPConnection *conn) const {
    DBG_MSG("%s", resource);

    HTTPRestData *d= new HTTPRestData();
    char *host;

    // The host field string can have junk at the end...
    host= (char*) conn->getField("Host");
    for (char *p= host; *p != 0; p++) {
        if(!isprint(*p)) {
            *p= 0;
        }
    }

    if (strcmp(resource, "/") == 0) {
        strcat(d->response, "hello word");
    } else if (strcmp(resource, "/format") == 0) {
        bool ok = FileSystem_MkFs();
        strcat(d->response, ok?"OK":"Failed");
    } else if (strstr(resource, "/mkdir?") == resource) {
        const char* dirname = resource+7;
        DBG_MSG("Making dir [%s]", dirname);
        bool ok = FileSystem_MkDir(dirname);
        strcat(d->response, ok?"OK":"Failed");
    } else return HTTP_NotFound;

    //conn->setHeaderFields("Content-Type: application/json");
    conn->setLength(strlen(d->response));
    conn->data= d;

    return HTTP_OK;
}

HTTPStatus HTTPRestHandler::doPost(char *resource, HTTPConnection *conn) const {
    DBG_MSG("%s", resource);
    return HTTP_NotFound;
}

void HTTPRestHandler::reg(HTTPServer *server) {
    server->registerField("Host");
}

HTTPStatus HTTPRestHandler::init(HTTPConnection *conn) const {
    char *resource=  conn->getURL() + prefixLength;
    switch (conn->getType()) {
        case GET:
            return doGet(resource, conn);

        case POST:
            return doPost(resource, conn);
    }
    return HTTP_BadRequest;
}


HTTPHandle HTTPRestHandler::data(HTTPConnection *conn, void *data, int len) const {
    //printf("REST data()\n");
    return HTTP_SuccessEnded;
}

HTTPHandle HTTPRestHandler::send(HTTPConnection *conn, int maxData) const {
    HTTPRestData *d= static_cast<HTTPRestData*>(conn->data);
    if(d){
        const char *str= d->response;
        int len= strlen(str);
        DBG_MSG("REST send: %d [%s]", len, str);
        conn->write((void*)str, len);
    }
    return HTTP_SuccessEnded;
}
