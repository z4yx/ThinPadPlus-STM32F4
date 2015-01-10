#include "ff.h"
#include "fs.h"
#include "httpd.h"
#include "lwip/pbuf.h"
#include "common.h"
#include <string.h>
#include <stdio.h>

static FIL opened_files[16];
static uint16_t is_used;

#define MAX_URI_SIZE 256

static void* current_post_conn;
static FIL   current_post_file;
static char  current_post_uri[MAX_URI_SIZE];
static uint8_t current_post_is_opened;

int fs_open_custom(struct fs_file *file, const char *name)
{
    FILINFO fno;
    FRESULT res;

    DBG_MSG("name=%s", name);

    for (int i = 0; i < 16; ++i)
    {
        if(!(is_used&(1<<i))){
            res = f_stat(name, &fno);
            if(res == FR_OK){
                if(fno.fsize == 0)
                    return 0;
                res = f_open(&opened_files[i], name, FA_READ);
                if(res == FR_OK){
                    is_used |= (1<<i);

                    file->data = NULL;
                    file->index = 0;
                    file->http_header_included = 0;
                    file->len = fno.fsize;
                    file->pextension = (void*)(i+1);

                    return 1;
                }
            }
            break;
        }
    }
    return 0;
}
void fs_close_custom(struct fs_file *file)
{
    if(!file->pextension)
        return;
    int i = (int)file->pextension - 1;
    f_close(&opened_files[i]);
    is_used ^= (1<<i);
}

int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
    UINT cnt;

    if(!file->pextension)
        return FS_READ_EOF;
    if(file->index >= file->len) {
        return FS_READ_EOF;
    }
    int i = (int)file->pextension - 1;
    f_read(&opened_files[i], buffer, count, &cnt);
    file->index += cnt;
    return cnt;
}


/** Called when a POST request has been received. The application can decide
 * whether to accept it or not.
 *
 * @param connection Unique connection identifier, valid until httpd_post_end
 *        is called.
 * @param uri The HTTP header URI receiving the POST request.
 * @param http_request The raw HTTP request (the first packet, normally).
 * @param http_request_len Size of 'http_request'.
 * @param content_len Content-Length from HTTP header.
 * @param response_uri Filename of response file, to be filled when denying the
 *        request
 * @param response_uri_len Size of the 'response_uri' buffer.
 * @param post_auto_wnd Set this to 0 to let the callback code handle window
 *        updates by calling 'httpd_post_data_recved' (to throttle rx speed)
 *        default is 1 (httpd handles window updates automatically)
 * @return ERR_OK: Accept the POST request, data may be passed in
 *         another err_t: Deny the POST request, send back 'bad request'.
 */
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd)
{
    FRESULT res;

    if(current_post_conn != NULL){
        strncpy(response_uri, "/404.html", response_uri_len);
        return ERR_VAL;
    }
    strncpy(current_post_uri, uri, MAX_URI_SIZE);
    current_post_conn = connection;
    current_post_is_opened = 0;
    *post_auto_wnd = 0;
    return ERR_OK;
}

/** Called for each pbuf of data that has been received for a POST.
 * ATTENTION: The application is responsible for freeing the pbufs passed in!
 *
 * @param connection Unique connection identifier.
 * @param p Received data.
 * @return ERR_OK: Data accepted.
 *         another err_t: Data denied, http_post_get_response_uri will be called.
 */
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
    if(current_post_conn != connection){
        return ERR_VAL;
    }
    FRESULT res;
    if(!current_post_is_opened){
        DBG_MSG("Opening %s for writing", current_post_uri);
        res = f_open(&current_post_file, current_post_uri, FA_WRITE | FA_CREATE_ALWAYS);
        if(res != FR_OK){
            return ERR_VAL;
        }
        current_post_is_opened = 1;
    }
    uint16_t length = p->tot_len;
    while(p){
        UINT cnt;
        f_write(&current_post_file, p->payload, p->len, &cnt);
        p = p->next;
    }
    DBG_MSG("call httpd_post_data_recved(%d)", length);
    httpd_post_data_recved(connection, length);
    return ERR_OK;
}

/** Called when all data is received or when the connection is closed.
 * The application must return the filename/URI of a file to send in response
 * to this POST request. If the response_uri buffer is untouched, a 404
 * response is returned.
 *
 * @param connection Unique connection identifier.
 * @param response_uri Filename of response file, to be filled when denying the request
 * @param response_uri_len Size of the 'response_uri' buffer.
 */
void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
    if(current_post_conn == connection){
        INFO_MSG("Done");
        f_close(&current_post_file);
        current_post_conn = NULL;
    }
    strncpy(response_uri, "", response_uri_len);
}
