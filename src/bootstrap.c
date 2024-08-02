#include "qbittorrent.c"
#include "shared.c"
#include <cJSON.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <cwalk.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <windows.h>

void hooktree(char *szFileName, char *hooker, char *cmdline, int pid,
              char *hash) {
    sprintf(cmdline, "%s\\%s \"%i\" %s", szFileName, hooker, pid, hash);
    system(cmdline);
    Sleep(1500);

    PROCESSENTRY32 pe;
    memset(&pe, 0, sizeof(PROCESSENTRY32));
    pe.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(hSnap, &pe)) {
        BOOL bContinue = TRUE;
        while (bContinue) {
            if (pe.th32ParentProcessID == pid) {
                sprintf(cmdline, "%s\\%s \"%lu\" %s", szFileName, hooker,
                        pe.th32ProcessID, hash);
                system(cmdline);
            }
            bContinue = Process32Next(hSnap, &pe);
        }
    } else {
        printf("OLOLOB\n");
    }
}
size_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
char *find_torrent(char *base) {
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

            char *content_path =
                cJSON_GetObjectItemCaseSensitive(torrent, "content_path")
                    ->valuestring;
#ifdef WINE
            char *newname = malloc(strlen(content_path) + 3);
            strcpy(newname + 2, content_path);
            newname[0] = 'Z';
            newname[1] = ':';
            int i = 0;
            for (i = 2; i < strlen(newname); i++) {
                if (newname[i] == '/')
                    newname[i] = '\\';
            }
            if (newname[i - 1] == '\\')
                newname[i - 1] = 0;
            newname[i] = 0;
            content_path = newname;
#endif
            printf("%s %s\n", content_path, base);
            if (strcmp(content_path, base) == 0) {
                return cJSON_GetObjectItemCaseSensitive(torrent, "hash")
                    ->valuestring;
            }
        }
        /*cJSON_Delete(json);*/
        curl_easy_cleanup(curl);
        if (buf.buf != NULL) {
            free(buf.buf);
        }
        return NULL;
    } else {
        return NULL;
    }
}
int main(int argc, char **argv) {
    STARTUPINFOA sinfo = {0};
    PROCESS_INFORMATION pinfo = {0};
    char *exe = argv[1];
    size_t len = 0;
    char *base = malloc(512);
    char *cmdline = malloc(512);
    if (exe[0] == '"') {
        len = strlen(exe);
        for (int i = len - 1; i >= 0; i--) {
            if (exe[i] == '"') {
                exe[i] = '\0';
                break;
            }
        }
        exe = &exe[1];
    }
    sprintf(cmdline, "\"%s\"", exe);
    strcpy(base, exe);
    cwk_path_set_style(cwk_path_guess_style(base));
    cwk_path_get_dirname(base, &len);
    if (len > 1 && (base[len - 1] == '/' || base[len - 1] == '\\'))
        base[len - 1] = 0;
    base[len] = 0;

    printf("'%s' '%s'\n", exe, base);
    char *torrent_hash = find_torrent(base);
    printf("hash: %s\n", torrent_hash);
    if (torrent_hash == NULL) {
        printf("Could find torrent\n");
        exit(1);
    }
    char *files_url = malloc(512);
    sprintf(files_url, "http://127.0.0.1:8080/api/v2/torrents/files?hash=%s",
            torrent_hash);
    cJSON *files = get_files(files_url);
    cJSON *file = NULL;
    char *basename = NULL;
    size_t blen = 0;
    ensure_seq_dl(torrent_hash);

    cwk_path_get_basename(exe, &basename, &blen);
    int index = -1;
    int i = 0;
    double progress = 0;
    cJSON_ArrayForEach(file, files) {
        char *name =
            cJSON_GetObjectItemCaseSensitive(file, "name")->valuestring;
        char *namebase = NULL;
        cwk_path_get_basename(name, &namebase, &blen);
        if (strcmp(basename, namebase) == 0) {
            index = i;
            printf("%s %s %i %i\n", basename, namebase, i,
                   cJSON_GetObjectItemCaseSensitive(file, "index")->valueint);
            progress =
                cJSON_GetObjectItemCaseSensitive(file, "progress")->valuedouble;
            break;
        }
        i += 1;
    }
    if (progress < 1.0) {
        printf("Waiting for setup to finish downloading...\n");
        set_max_priority(torrent_hash, index, 7);
        while (progress < 1.0) {
            Sleep(1000);
            cJSON_Delete(files);
            files = get_files(files_url);
            file = cJSON_GetArrayItem(files, index);
            progress =
                cJSON_GetObjectItemCaseSensitive(file, "progress")->valuedouble;
        }
        pause(torrent_hash);
        Sleep(1000);
    }

    CreateProcessA(exe, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL,
                   base, &sinfo, &pinfo);
    resume(torrent_hash);

    BOOL is32 = FALSE;
    IsWow64Process(pinfo.hProcess, &is32);
    char *hooker = NULL;
    if (is32) {
        hooker = "hooker32.exe";
    } else {
        hooker = "hooker64.exe";
    }
    printf("starting %s\n", hooker);
    TCHAR szFileName[MAX_PATH];
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    cwk_path_set_style(cwk_path_guess_style(szFileName));
    cwk_path_get_dirname(szFileName, &len);
    szFileName[len] = 0;
    SetCurrentDirectory(szFileName);
    hooktree(szFileName, hooker, cmdline, pinfo.dwProcessId, torrent_hash);
}
