#include <windows.h>
#pragma once
struct response_buf {
  char *buf;
  size_t size;
};
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
