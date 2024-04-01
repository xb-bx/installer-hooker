# Installer hook

Allows you to start installing game from torrent as soon as installer is downloaded.
Works only with qbittorrent

It works via injecting code into setup.exe(or whatever the name of installer is) and when it tries to open file that isnt downloaded yet it sleeps until the file is downloaded.

# Build requirements 
- CMake
- Git

# Requirements 
- Qbittorent with WebUI enabled on port 8080 and enabled "Bypass authentication for clients on localhost"

# How to use

1. Start downloading torrent 
2. As soon as setup.exe donwloaded pause torrent
3. Drag-n-drop setup.exe on bootstrap.exe
4. Unpause torrent






