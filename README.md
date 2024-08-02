# Installer hook

Allows you to start installing game from torrent as soon as installer is downloaded.
Works only with qbittorrent

It works via injecting code into setup.exe(or whatever the name of installer is) and when it tries to open file that isnt downloaded yet it sleeps until the file is downloaded.

# Build requirements
## Windows:
    - MSVC
    - CMake
    - Git
## Linux:
    - [MSVC](https://github.com/mstorsjo/msvc-wine)
    - Make
    - Git

# Building
## Builing on linux
- Building for 1 architecture
    - msvc/bin/x(64/86) must be in PATH
    - `make all`
- Building for both architectures. 
    ```sh
    MSVC_X32=<path-to-msvc-32bit-binaries> MSVC_X64=<path-to-msvc-64bit-binaries> make all-arch
    ```
    if MSVC_X\* is not defined the default value of /opt/msvc/bin/x\* will be used
## Building on Windows
```powershell
PS> ./fetch-libs.ps1 
PS> ./build-x64.ps1 && ./build-x64.ps1
```

# Requirements 
- Qbittorent with WebUI enabled on port 8080 and enabled "Bypass authentication for clients on localhost"

# How to use

1. Start downloading torrent 
2. Drag-n-drop setup.exe on bootstrap.exe
3. Unpause torrent
4. Wait for install to complete
