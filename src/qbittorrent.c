#include "shared.h"
#include <cJSON.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <cwalk.h>

void resume(char *torrent_hash) {
  CURL *curl;
  CURLcode rescode;
  curl = curl_easy_init();
  if (curl) {
    struct response_buf buf = {0};
    char params[512];
    params[sprintf(params, "hashes=%s", torrent_hash)] = 0;
    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://127.0.0.1:8080/api/v2/torrents/resume");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    rescode = curl_easy_perform(curl);
    if (rescode != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(rescode));

    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
  }
}
void pause(char *torrent_hash) {
  CURL *curl;
  CURLcode rescode;
  curl = curl_easy_init();
  if (curl) {
    struct response_buf buf = {0};
    char params[512];
    params[sprintf(params, "hashes=%s", torrent_hash)] = 0;
    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://127.0.0.1:8080/api/v2/torrents/pause");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    rescode = curl_easy_perform(curl);
    if (rescode != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(rescode));

    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
  }
}
void set_max_priority(char *torrent_hash, int index, int prior) {
  CURL *curl;
  CURLcode rescode;
  curl = curl_easy_init();
  if (curl) {
    struct response_buf buf = {0};
    char params[512];
    params[sprintf(params, "hash=%s&id=%i&priority=%i", torrent_hash, index,
                   prior)] = 0;
    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://127.0.0.1:8080/api/v2/torrents/filePrio");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    rescode = curl_easy_perform(curl);
    if (rescode != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(rescode));

    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
  }
}
cJSON *get_files(char *files_url) {
  CURL *curl;
  CURLcode rescode;
  curl = curl_easy_init();
  if (curl) {
    struct response_buf buf = {0};
    curl_easy_setopt(curl, CURLOPT_URL, files_url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    rescode = curl_easy_perform(curl);
    if (rescode != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(rescode));

    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
    return json;
  } else {
    printf("couldn't fetch files\n");
    return NULL;
  }
}

BOOL check_torrent(char *torrent_hash, char **base_path) {
  CURL *curl;
  CURLcode rescode;
  curl = curl_easy_init();
  if (curl) {
    struct response_buf buf = {0};
    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://127.0.0.1:8080/api/v2/torrents/info");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    rescode = curl_easy_perform(curl);
    if (rescode != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(rescode));

    cJSON *json = cJSON_Parse(buf.buf);
    cJSON *torrent = NULL;
    cJSON_ArrayForEach(torrent, json) {
      char *hash =
          cJSON_GetObjectItemCaseSensitive(torrent, "hash")->valuestring;
      if (strcmp(hash, torrent_hash) == 0) {
        printf("Found\n");
        *base_path = malloc(512);
        (*base_path)[0] = 0;
        strcpy(*base_path,
               cJSON_GetObjectItemCaseSensitive(torrent, "content_path")
                   ->valuestring);
        size_t len;
        cwk_path_get_dirname(*base_path, &len);
        (*base_path)[len] = 0;
#ifdef WINE
        char *newname = malloc(strlen(*base_path) + 3);
        strcpy(newname + 2, *base_path);
        newname[0] = 'Z';
        newname[1] = ':';
        int i = 0;
        for (i = 2; i < strlen(newname); i++) {
          if (newname[i] == '/')
            newname[i] = '\\';
        }
        if (newname[i - 1] == '\\') newname[i - 1] = 0;
        newname[i] = 0;
        free(*base_path);
        *base_path = newname;
#endif
        printf("%i %s\n", len, *base_path);
        cJSON_Delete(json);
        curl_easy_cleanup(curl);
        if (buf.buf != NULL) {
          free(buf.buf);
        }
        return TRUE;
      }
    }
    cJSON_Delete(json);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
    return FALSE;
  } else {
    return FALSE;
  }
}
