#include <stdio.h>
#include <windows.h>

int main(int argc, char **argv) {
  int pid = atoi(argv[1]);
  HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  void *loadlib =
      GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
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
  void *set_hash = GetProcAddress(hookdll, set_hook_name);
  dllpathbuf = buf;
  void *dllpath = VirtualAllocEx(process, NULL, strlen(dllpathbuf),
                                 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  void *hash = VirtualAllocEx(process, NULL, 512, MEM_RESERVE | MEM_COMMIT,
                              PAGE_READWRITE);
  WriteProcessMemory(process, dllpath, dllpathbuf, strlen(dllpathbuf), NULL);
  WriteProcessMemory(process, hash, argv[2], strlen(argv[2]), NULL);
  CreateRemoteThread(process, NULL, 0, loadlib, dllpath, 0, NULL);
  Sleep(500);
  CreateRemoteThread(process, NULL, 0, set_hash, hash, 0, NULL);
  return 0;
}
