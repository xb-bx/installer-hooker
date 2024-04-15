#include "qbittorrent.c"
#include "shared.c"
#include <cJSON.h>
#include <converter.h>
#include <stdint.h>
#define CURL_STATICLIB
#include "handles_map.c"
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
char oldRead[16];
char newRead[16];
char *addrRead;
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
    ensure_seq_dl(torrent_hash);
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
HandlesMap *map = NULL;
PIECE_STATE *pieces = NULL;
int pieces_count;
int piece_size = 0;
void register_file(char *filename, HANDLE h) {
  if (map == NULL) {
    map = map_new();
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
  if (pieces == NULL) {
    pieces_count = get_pieces_count(torrent_hash, &piece_size);
    pieces = malloc(sizeof(PIECE_STATE) * pieces_count);
  }
  cJSON *files = get_files(files_url);
  cJSON *file = NULL;
  char buffer[1024];
  BOOL found = FALSE;
  int i = 0;
  cJSON_ArrayForEach(file, files) {
    char *name = cJSON_GetObjectItemCaseSensitive(file, "name")->valuestring;
    cwk_path_set_style(cwk_path_guess_style(base_path));
    cwk_path_join(base_path, name, buffer, 2048);
    if (strcmp(filename, buffer) == 0) {
      found = TRUE;
      break;
    }
    i++;
  }
  if (!found) {
    return;
  }
  cJSON *piece_range = cJSON_GetObjectItemCaseSensitive(file, "piece_range");
  int start = cJSON_GetArrayItem(piece_range, 0)->valueint;
  int end = cJSON_GetArrayItem(piece_range, 1)->valueint;
  char *name = malloc(strlen(filename));
  cJSON_Delete(files);
  strcpy(name, filename);
  map_add(map, (FileInfo){.name = name,
                          .index = i,
                          .finished = FALSE,
                          .h = h,
                          .piece_start = start,
                          .piece_end = end,
                          .last_downloaded_piece = 0});
}

BOOL __declspec(dllexport) __stdcall ReadFileHook(HANDLE hFile, LPVOID lpBuffer,
                                                  DWORD nNumberOfBytesToRead,
                                                  LPDWORD lpNumberOfBytesRead,
                                                  LPOVERLAPPED lpOverlapped) {
  FileInfo *file = map_get(map, hFile);
  if (file != NULL && !file->finished) {
    int index;
    if (!file->finished) {
      uint32_t high_dword = 0;
      uint32_t low_dword = SetFilePointer(hFile, 0, &high_dword, FILE_CURRENT);
      int64_t pos = (int64_t)high_dword << 32 | (int64_t)low_dword;
      int start_piece = file->piece_start + pos / piece_size;
      int end_piece = start_piece + nNumberOfBytesToRead / piece_size + 16;
      if (nNumberOfBytesToRead % piece_size > 0) {
        end_piece++;
      }
      if (end_piece > pieces_count) {
        end_piece = pieces_count;
      }
      if (!(file->last_downloaded_piece >= end_piece)) {
        BOOL finished = FALSE;
        while (!finished) {
          get_pieces(torrent_hash, pieces);

          printf("waitiuiioing for %s %lli %i %i %i %i %i\n", file->name, pos,
                 piece_size, file->piece_start, file->piece_end, start_piece,
                 end_piece);
          /*printf("pieces: ");*/
          for (int i = start_piece; i <= end_piece && i < pieces_count; i++) {
            /*printf("%i ", pieces[i]);*/
            if (pieces[i] != DOWNLOADED) {
              file->last_downloaded_piece = i - 1;
              goto wait;
            }
          }
          for (int i = file->piece_start; i < pieces_count && i < file->piece_end;
               i++) {
            file->last_downloaded_piece = i;
            if (pieces[i] != DOWNLOADED) {
              file->last_downloaded_piece = i - 1;
              break;
            }
          }
          Sleep(1000);
          break;
        wait:
          printf("waiting for %s %i %i %i %i\n", file->name, file->piece_start,
                 file->piece_end, start_piece, end_piece);
          Sleep(5000);
        }
        if(end_piece == file->piece_end) {
            file->finished = TRUE;
            pause(torrent_hash);
            set_max_priority(torrent_hash, file->index, 0);
            Sleep(1000);
            resume(torrent_hash);
        }
      }
    }
  }
  for (int i = 0; i < 16; i++)
    addrRead[i] = oldRead[i];
  BOOL res = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                      lpNumberOfBytesRead, lpOverlapped);
  for (int i = 0; i < 16; i++)
    addrRead[i] = newRead[i];
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

  for (int i = 0; i < 16; i++)
    addrW[i] = oldW[i];
  HANDLE res = CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                           lpSecurityAttributes, dwCreationDisposition,
                           dwFlagsAndAttributes, hTemplateFile);
  register_file(utf8filename, res);
  for (int i = 0; i < 16; i++)
    addrW[i] = newW[i];
  free(utf8filename);
  return res;
}

void __declspec(dllexport)
    create_hook(char *old, char *new, char *addr, void *hook) {
  int offset = 0;
#if defined _M_IX86
  addr[offset + 0] = 0xb8;
  *((void **)&addr[offset + 1]) = hook;
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

    addrRead = GetProcAddress(GetModuleHandle("kernel32.dll"), "ReadFile");
    for (int i = 0; i < 16; i++)
      oldRead[i] = addrRead[i];
    old_protect = 0;
    if (!VirtualProtect(addrRead, 4096, PAGE_EXECUTE_READWRITE, &old_protect)) {
      printf("error\n");
    }
    create_hook(oldRead, newRead, addrRead, ReadFileHook);
    for (int i = 0; i < 16; i++)
      newRead[i] = addrRead[i];
    /*addrWKB = GetProcAddress(GetModuleHandle("kernelbase.dll"),
     * "CreateFileW");*/
    /*for (int i = 0; i < 16; i++)*/
    /*oldWKB[i] = addrWKB[i];*/
    /*old_protect = 0;*/
    /*if (!VirtualProtect(addrWKB, 4096, PAGE_EXECUTE_READWRITE, &old_protect))
     * {*/
    /*printf("error\n");*/
    /*}*/
    /*create_hook(oldWKB, newWKB, addrWKB, hookWKB);*/
    /*for (int i = 0; i < 16; i++)*/
    /*newWKB[i] = addrWKB[i];*/

    /*addrA = GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileA");*/
    /*for (int i = 0; i < 16; i++)*/
    /*oldA[i] = addrA[i];*/
    /*old_protect = 0;*/
    /*if (!VirtualProtect(addrA, 4096, PAGE_EXECUTE_READWRITE, &old_protect))
     * {*/
    /*printf("error\n");*/
    /*}*/
    /*create_hook(oldA, newA, addrA, hookA);*/
    /*for (int i = 0; i < 16; i++)*/
    /*newA[i] = addrA[i];*/

    /*printf("OK\n");*/
  }
  return TRUE;
}
