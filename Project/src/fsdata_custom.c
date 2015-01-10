#include "ff.h"
#include "fs.h"
#include "lwip/pbuf.h"
#include "common.h"
#include <string.h>
#include <stdio.h>

static FIL opened_files[16];
static uint16_t is_used;

static void* current_post_conn;
static FIL   current_post_file;

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
                DBG_MSG("File found len=%d", fno.fsize);
                if(fno.fsize == 0)
                    return 0;
                DBG_MSG("before f_open");
                res = f_open(&opened_files[i], name, FA_READ);
                DBG_MSG("f_open returned %d", res);
                if(res == FR_OK){
                    is_used |= (1<<i);

                    DBG_MSG("opened_files %d is used", i);
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
    DBG_MSG("closing file %d", i);
    f_close(&opened_files[i]);
    is_used ^= (1<<i);
}

int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
    UINT cnt;

    DBG_MSG("index=%d count=%d", file->index, count);
    if(!file->pextension)
        return FS_READ_EOF;
    if(file->index >= file->len) {
        return FS_READ_EOF;
    }
    int i = (int)file->pextension - 1;
    DBG_MSG("reading file %d", i);
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

    DBG_MSG("connection=%u uri=%s content_len=%d",
        connection, uri, content_len);
    if(current_post_conn != NULL){
        strncpy(response_uri, "/404.html", response_uri_len);
        return ERR_VAL;
    }
    res = f_open(&current_post_file, uri+1, FA_WRITE | FA_CREATE_ALWAYS);
    if(res == FR_OK){
        current_post_conn = connection;
        return ERR_OK;
    }
    return ERR_VAL;
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
    DBG_MSG("connection=%u",
        connection);
    if(current_post_conn != connection){
        return ERR_VAL;
    }
    while(p){
        UINT cnt;
        DBG_MSG("payload len=%d", p->len);
        f_write(&current_post_file, p->payload, p->len, &cnt);
        p = p->next;
    }
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
    DBG_MSG("connection=%u",
        connection);
    if(current_post_conn == connection){
        f_close(&current_post_file);
        current_post_conn = NULL;
    }
    strncpy(response_uri, "", response_uri_len);
}
