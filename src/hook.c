#include "qbittorrent.c"
#include "shared.c"
#include <cJSON.h>
#include <converter.h>
#define CURL_STATICLIB
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

char **downloaded = NULL;
int downloadcount = 0;

double __declspec(dllexport) check_file(char *filename, int *index) {
  cJSON *files = get_files(files_url);
  cJSON *file = NULL;
  char buffer[2048] = {0};
  double res = -1.0;
  if (downloaded == NULL) {
    downloaded = malloc(sizeof(char *) * cJSON_GetArraySize(files));
  }
  cJSON_ArrayForEach(file, files) {
    char *name = cJSON_GetObjectItemCaseSensitive(file, "name")->valuestring;
    cwk_path_set_style(cwk_path_guess_style(base_path));
    cwk_path_join(base_path, name, buffer, 2048);
    if (strcmp(filename, buffer) == 0) {
      res = cJSON_GetObjectItemCaseSensitive(file, "progress")->valuedouble;
      *index = cJSON_GetObjectItemCaseSensitive(file, "index")->valueint;
      break;
    }
  }
  cJSON_Delete(files);
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
  if (is_downloaded(lpFileName)) {
    return;
  }
  if (torrent_hash == NULL) {
    while (torrent_hash == NULL) {
      Sleep(100);
    }
  }
  if (files_url == NULL) {
    if (!check_torrent(torrent_hash, &base_path)) {
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
      set_max_priority(torrent_hash, index, 7);
      prioritySet = true;
    }
    printf("%s %f\n", lpFileName, progress);
    if (!(progress <= 0 || progress >= 1)) {
      Sleep(3000);
    }
  } while (progress >= 0.0 && progress < 1.0);
  if (prioritySet) {
    pause(torrent_hash);
    Sleep(5000);
    resume(torrent_hash);
    set_max_priority(torrent_hash, index, 0);
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

    /*addrAttrsW =*/
        /*GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFileAttributesW");*/
    /*for (int i = 0; i < 16; i++)*/
      /*attrsOldW[i] = addrAttrsW[i];*/
    /*old_protect = 0;*/
    /*if (!VirtualProtect(addrAttrsW, 4096, PAGE_EXECUTE_READWRITE,*/
                        /*&old_protect)) {*/
      /*printf("error\n");*/
    /*}*/
    /*create_hook(attrsOldW, attrsNewW, addrAttrsW, hookFileAttrsW);*/
    /*for (int i = 0; i < 16; i++)*/
      /*attrsNewW[i] = addrAttrsW[i];*/

    printf("OK\n");
  }
  return TRUE;
}
