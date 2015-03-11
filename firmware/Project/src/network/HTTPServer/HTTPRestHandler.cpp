#include "HTTPRestHandler.h"
#include "stdio.h"
// #include "TemperatureSensor.h"
// #include "RGBLed.h"

// extern TemperatureSensor sensor;
// extern RGBLed rgb;

#define in_range(c, lo, up)  ((u8_t)c >= lo && (u8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)

HTTPStatus HTTPRestHandler::doGet(char *resource, HTTPConnection *conn) const {
    printf("HTTPRestHandler GET: %s\n", resource);

    HTTPRestData *d= new HTTPRestData();
    char *host;
    char buffer[20] = "hello";

    // The host field string can have junk at the end...
    host= (char*) conn->getField("Host");
    for (char *p= host; *p != 0; p++) {
        if(!isprint(*p)) {
            *p= 0;
        }
    }

    if (strcmp(resource, "/") == 0) {
        strcat(d->response, "{\"temperature\":\"http://");
        strcat(d->response, host);
        strcat(d->response, conn->getURL());
        strcat(d->response, "temperature\",\"rgbled\":\"http://");
        strcat(d->response, host);
        strcat(d->response, conn->getURL());
        strcat(d->response, "rgbled\"}");
    } else if (strcmp(resource, "/temperature") == 0) {
        // sensor.measure();
        // sprintf(buffer, "%f", sensor.getKelvin());
    
        strcat(d->response, "{\"realtime-uri\":\"ws://");
        strcat(d->response, host);
        strcat(d->response, "/ws/\",\"format\":\"text/csv\",\"fields\":\"ADC,resistence,temperature\",\"calibration\":\"http:/");
        strcat(d->response, conn->getURL());
        strcat(d->response, "/calibration\",");
        strcat(d->response, "\"units\":\"kelvin\",\"value\":");
        strcat(d->response, buffer);
        strcat(d->response, "}");
    } else if (strcmp(resource, "/temperature/calibration") == 0) {
        strcat(d->response, "{TODO}");
    } else if (strcmp(resource, "/rgbled") == 0) {
        strcat(d->response, "{\"web-socket-uri\":\"ws://");
        strcat(d->response, host);
        strcat(d->response, "/ws/\",\"format\":\"text/csv\",\"fields\":\"red,green,blue\"}");
    } else return HTTP_NotFound;

    //conn->setHeaderFields("Content-Type: application/json");
    conn->setLength(strlen(d->response));
    conn->data= d;

    //printf("json: [ %s ]\n", static_cast<HTTPRestData*>(conn->data)->response);
    return HTTP_OK;
}

HTTPStatus HTTPRestHandler::doPost(char *resource, HTTPConnection *conn) const {
    printf("HTTPRestHandler POST: %s\n",  resource);
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
    const char *str= d->response;
    int len= strlen(str);
    printf("REST send: %d [ %s ]\n", len, str);
    conn->write((void*)str, len);
    return HTTP_SuccessEnded;
}
