#ifndef SERIAL_REDIRECT_H__
#define SERIAL_REDIRECT_H__

#include "HTTPServer.h"
#include "thinpad.h"

#ifdef __cplusplus
extern "C" {
#endif

void SerialRedirect_Init(HTTPServer* httpd);
void SerialRedirect_Close(void);
void SerialRedirect_Open(int baud);
void SerialRedirect_ToThinpad(uint8_t* data, int len);

void SerialRedirect_UsartInterrupt(void);

#ifdef __cplusplus
};
#endif

#endif

