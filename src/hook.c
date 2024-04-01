#include "shared.c"
#include <cJSON.h>
#include <converter.h>
#include <curl/curl.h>
#include <cwalk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <windows.h>
#include <winuser.h>

char attrsOldW[16];
char attrsNewW[16];
char *addrAttrsW;
char oldW[16];
char newW[16];
char *addrW;
char oldWKB[16];
char newWKB[16];
char *addrWKB;
char oldA[16];
char newA[16];
char *addrA;

char *torrent_hash;
char *files_url = NULL;
char *base_path;

void resume() {
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

    /* always cleanup */
    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
  }
}
void pause() {
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

    /* always cleanup */
    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
  }
}
void set_max_priority(int index, int prior) {
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

    /* always cleanup */
    cJSON *json = cJSON_Parse(buf.buf);
    curl_easy_cleanup(curl);
    if (buf.buf != NULL) {
      free(buf.buf);
    }
  }
}
cJSON *files = NULL;
cJSON *get_files() {
  if (files == NULL) {
    CURL *curl;
    CURLcode rescode;
    curl = curl_easy_init();
    if (curl) {
      struct response_buf buf = {0};
      curl_easy_setopt(curl, CURLOPT_URL, files_url);
      /*curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);*/
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

      rescode = curl_easy_perform(curl);
      if (rescode != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(rescode));

      /* always cleanup */
      cJSON *json = cJSON_Parse(buf.buf);
      curl_easy_cleanup(curl);
      if (buf.buf != NULL) {
        free(buf.buf);
      }
      return json;
    } else {
      return NULL;
    }
  } else {
    return files;
  }
}

BOOL check_torrent() {
  CURL *curl;
  CURLcode rescode;
  curl = curl_easy_init();
  if (curl) {
    struct response_buf buf = {0};
    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://127.0.0.1:8080/api/v2/torrents/info");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    /*curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);*/
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    rescode = curl_easy_perform(curl);
    if (rescode != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(rescode));

    /* always cleanup */
    cJSON *json = cJSON_Parse(buf.buf);
    cJSON *torrent = NULL;
    cJSON_ArrayForEach(torrent, json) {
      char *hash =
          cJSON_GetObjectItemCaseSensitive(torrent, "hash")->valuestring;
      if (strcmp(hash, torrent_hash) == 0) {
        printf("Found\n");
        base_path = malloc(512);
        base_path[0] = 0;
        strcpy(base_path,
               cJSON_GetObjectItemCaseSensitive(torrent, "content_path")
                   ->valuestring);
        size_t len;
        cwk_path_get_dirname(base_path, &len);
        base_path[len] = 0;
        printf("%i %s\n", len, base_path);
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
char **downloaded = NULL;
int downloadcount = 0;
double __declspec(dllexport) check_file(char *filename, int *index) {
  cJSON *files = get_files();
  cJSON *file = NULL;
  char buffer[2048];
  double res = -1.0;
  if (downloaded == NULL) {
    downloaded = malloc(sizeof(char *) * cJSON_GetArraySize(files));
  }
  cJSON_ArrayForEach(file, files) {
    char *name = cJSON_GetObjectItemCaseSensitive(file, "name")->valuestring;
    cwk_path_join(base_path, name, buffer, 2048);
    if (strcmp(filename, buffer) == 0) {
      res = cJSON_GetObjectItemCaseSensitive(file, "progress")->valuedouble;
      *index = cJSON_GetObjectItemCaseSensitive(file, "index")->valueint;
      break;
    }
  }
  return res;
}
BOOL is_downloaded(char *filename) {
  if (downloaded == NULL)
    return FALSE;
  for (int i = 0; i < downloadcount; i++) {
    if (strcmp(downloaded[i], filename) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}
void __declspec(dllexport) wait_for_file(char *lpFileName) {
  if (is_downloaded(lpFileName))
    return;
  if (torrent_hash == NULL) {
    while (torrent_hash == NULL)
      Sleep(100);
  }
  if (files_url == NULL) {
    if (!check_torrent()) {
      printf("unknown torrent");
      exit(1);
    }
    files_url = malloc(512);
    sprintf(files_url, "http://127.0.0.1:8080/api/v2/torrents/files?hash=%s",
            torrent_hash);
  }
  double progress = 0.0;
  bool prioritySet = false;
  int index = -1;
  do {
    progress = check_file(lpFileName, &index);
    if (index == -1) {
      break;
    }
    if (!prioritySet) {
      set_max_priority(index, 7);
      prioritySet = true;
    }
    printf("%s %f\n", lpFileName, progress);
    if (!(progress <= 0 || progress >= 1)) {
      Sleep(3000);
    }
  } while (progress >= 0.0 && progress < 1.0);
  if (prioritySet) {
    pause();
    Sleep(5000);
    resume();
    set_max_priority(index, 0);
    char **newitem = &downloaded[downloadcount++];
    *newitem = malloc(strlen(lpFileName));
    strcpy(*newitem, lpFileName);
  }
}
void __declspec(dllexport) __stdcall set_hash(char *hash) {
  torrent_hash = hash;
}
HANDLE __declspec(dllexport) __stdcall hookA(
    LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
  wait_for_file(lpFileName);
  for (int i = 0; i < 16; i++)
    addrA[i] = oldA[i];
  HANDLE res = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
                           lpSecurityAttributes, dwCreationDisposition,
                           dwFlagsAndAttributes, hTemplateFile);
  for (int i = 0; i < 16; i++)
    addrA[i] = newA[i];
  return res;
}

HANDLE __declspec(dllexport) __stdcall hookWKB(
    LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
  char *utf8filename = malloc(512);
  size_t len =
      utf16_to_utf8(lpFileName, lstrlenW(lpFileName), utf8filename, 512);
  utf8filename[len] = 0;
  wait_for_file(utf8filename);

  for (int i = 0; i < 16; i++)
    addrWKB[i] = oldWKB[i];
  HANDLE res = CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                           lpSecurityAttributes, dwCreationDisposition,
                           dwFlagsAndAttributes, hTemplateFile);
  for (int i = 0; i < 16; i++)
    addrWKB[i] = newWKB[i];
  free(utf8filename);
  return res;
}
HANDLE __declspec(dllexport) __stdcall hookW(
    LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
  char *utf8filename = malloc(512);
  size_t len =
      utf16_to_utf8(lpFileName, lstrlenW(lpFileName), utf8filename, 512);
  utf8filename[len] = 0;
  wait_for_file(utf8filename);

  for (int i = 0; i < 16; i++)
    addrW[i] = oldW[i];
  HANDLE res = CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                           lpSecurityAttributes, dwCreationDisposition,
                           dwFlagsAndAttributes, hTemplateFile);
  for (int i = 0; i < 16; i++)
    addrW[i] = newW[i];
  free(utf8filename);
  return res;
}
DWORD __declspec(dllexport) __stdcall hookFileAttrsW(LPCWSTR lpFileName) {

  char *utf8filename = malloc(512);
  size_t len =
      utf16_to_utf8(lpFileName, lstrlenW(lpFileName), utf8filename, 512);
  utf8filename[len] = 0;
  wait_for_file(utf8filename);
  for (int i = 0; i < 16; i++)
    addrAttrsW[i] = attrsOldW[i];
  DWORD res = GetFileAttributesW(lpFileName);
  for (int i = 0; i < 16; i++)
    addrAttrsW[i] = attrsNewW[i];
  free(utf8filename);
  return res;
}

/*int ___stdio_common_vfprintf(const char * restrict format, ... );*/
void __declspec(dllexport)
    create_hook(char *old, char *new, char *addr, void *hook) {
  int offset = 0;
#if defined _M_IX86
  addr[offset + 0] = 0xb8;
  *((int *)&addr[offset + 1]) = hook;
  addr[offset + 5] = 0xff;
  addr[offset + 6] = 0xe0;
#elif defined _M_X64
  addr[offset] = 0x48;
  addr[offset + 1] = 0xb8;
  *((void **)&addr[offset + 2]) = hook;
  addr[offset + 10] = 0xff;
  addr[offset + 11] = 0xe0;
#endif
}

BOOL __declspec(dllexport) WINAPI
    DllMain(HINSTANCE _hinstDLL, // handle to DLL module
            DWORD _fdwReason,    // reason for calling function
            LPVOID _lpReserved)  // reserved
{
  if (_fdwReason == DLL_PROCESS_ATTACH) {
    if (AllocConsole()) {
      freopen("CONIN$", "r", stdin);
      freopen("CONOUT$", "w", stdout);
      freopen("CONOUT$", "w", stderr);
    }
    addrW = GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileW");
    for (int i = 0; i < 16; i++)
      oldW[i] = addrW[i];
    DWORD old_protect = 0;
    if (!VirtualProtect(addrW, 4096, PAGE_EXECUTE_READWRITE, &old_protect)) {
      printf("error\n");
    }
    create_hook(oldW, newW, addrW, hookW);
    for (int i = 0; i < 16; i++)
      newW[i] = addrW[i];

    addrWKB = GetProcAddress(GetModuleHandle("kernelbase.dll"), "CreateFileW");
    for (int i = 0; i < 16; i++)
      oldWKB[i] = addrWKB[i];
    old_protect = 0;
    if (!VirtualProtect(addrWKB, 4096, PAGE_EXECUTE_READWRITE, &old_protect)) {
      printf("error\n");
    }
    create_hook(oldWKB, newWKB, addrWKB, hookWKB);
    for (int i = 0; i < 16; i++)
      newWKB[i] = addrWKB[i];

    addrA = GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileA");
    for (int i = 0; i < 16; i++)
      oldA[i] = addrA[i];
    old_protect = 0;
    if (!VirtualProtect(addrA, 4096, PAGE_EXECUTE_READWRITE, &old_protect)) {
      printf("error\n");
    }
    create_hook(oldA, newA, addrA, hookA);
    for (int i = 0; i < 16; i++)
      newA[i] = addrA[i];

    addrAttrsW =
        GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFileAttributesW");
    for (int i = 0; i < 16; i++)
      attrsOldW[i] = addrAttrsW[i];
    old_protect = 0;
    if (!VirtualProtect(addrAttrsW, 4096, PAGE_EXECUTE_READWRITE,
                        &old_protect)) {
      printf("error\n");
    }
    create_hook(attrsOldW, attrsNewW, addrAttrsW, hookFileAttrsW);
    for (int i = 0; i < 16; i++)
      attrsNewW[i] = addrAttrsW[i];

    printf("OK\n");
  }
  return TRUE;
}
