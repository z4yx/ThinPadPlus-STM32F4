#include "fs.h"
#include "common.h"
#include <string.h>

const char* custom_data = "Hello World";

int fs_open_custom(struct fs_file *file, const char *name)
{
    printf("call %s name=%s\r\n", __func__, name);
    file->data = NULL;
    file->len = 11;
    file->index = 0;
    file->pextension = NULL;
    file->http_header_included = 0;
}
void fs_close_custom(struct fs_file *file)
{

    printf("call %s\r\n", __func__);
}

int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
    printf("call %s index=%d count=%d\r\n", __func__, file->index, count);
    int left = file->len - file->index;
    if(file->index >= file->len) {
        return FS_READ_EOF;
    }
    if(count < left)
        left = count;
    memcpy(buffer, custom_data + file->index, left);
    file->index += left;
    return left;
}
