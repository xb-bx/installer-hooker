#include "shared.h"
#include <windows.h>
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    struct response_buf *mem = (struct response_buf *)userp;
    size_t realsize = size * nmemb;
    mem->buf = realloc(mem->buf, mem->size + realsize + 1);
    memcpy(&(mem->buf[mem->size]), buffer, realsize);
    mem->size += realsize;
    mem->buf[mem->size] = 0;
    return realsize;
}
