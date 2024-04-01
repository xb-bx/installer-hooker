#include<windows.h>
#include<stdio.h>
void printLastErr() {
    printf("%i\n", GetLastError());
}


int main(int argc, char **argv) {
    int pid = atoi(argv[1]);
    printf("%i\n", pid);
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    printf("%p %i\n", process, GetLastError());
    void* loadlib = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    printLastErr();
    printf("%p\n", loadlib);
#if defined _M_IX86
    char *dllpathbuf = "hook32.dll";
    char *set_hook_name = "_set_hash@4";
#elif defined _M_X64
    char *dllpathbuf = "hook64.dll";
    char *set_hook_name = "_set_hash@8";
#endif
    char *buf = malloc(512);
    GetFullPathName(dllpathbuf, 512, buf, NULL);
    HANDLE hookdll = LoadLibrary(buf);
    printf("%i\n", GetLastError()); 
    void *set_hash = GetProcAddress(hookdll, set_hook_name);
    printf("\n\ninjecting %s to %i and %p from %s in %p\n\n\n", dllpathbuf, pid, set_hash, buf, hookdll);
    dllpathbuf = buf;
    void *dllpath = VirtualAllocEx(process, NULL, strlen(dllpathbuf), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    void *hash = VirtualAllocEx(process, NULL, 512, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    printf("%p\n", hash);
    printLastErr();
    WriteProcessMemory(process, dllpath, dllpathbuf, strlen(dllpathbuf), NULL);
    printLastErr();
    WriteProcessMemory(process, hash, argv[2], strlen(argv[2]), NULL);
    printLastErr();
    CreateRemoteThread(process, NULL, 0, loadlib, dllpath, 0, NULL);
    Sleep(500);
    CreateRemoteThread(process, NULL, 0, set_hash, hash, 0, NULL);
    printLastErr();
    return 0;
}
